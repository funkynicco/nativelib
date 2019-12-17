using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.IO.Pipes;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace NativeLibAllocationTrace
{
    public abstract class RpcEvent : IDisposable
    {
        public void Dispose()
            => Dispose(true);

        protected virtual void Dispose(bool disposing) { }
    }

    public class ConnectedRpcEvent : RpcEvent
    {
    }

    public class DisconnectedRpcEvent : RpcEvent
    {
    }

    public class AddAllocationRpcEvent : RpcEvent
    {
        public double Time { get; private set; }

        public string Filename { get; private set; }

        public int Line { get; private set; }

        public string Function { get; private set; }

        public long Pointer { get; private set; }

        public long SizeOfPointerData { get; private set; }

        private readonly List<long> _stack = new List<long>();
        public IReadOnlyList<long> Stack => _stack;

        public static AddAllocationRpcEvent FromStream(BinaryReader br)
        {
            var ev = new AddAllocationRpcEvent();
            ev.Time = br.ReadDouble();
            ev.Filename = br.BaseStream.ReadNullTerminatedString(260);
            ev.Line = br.ReadInt32();
            ev.Function = br.BaseStream.ReadNullTerminatedString(64);
            ev.Pointer = br.ReadInt64();
            ev.SizeOfPointerData = br.ReadInt64();

            var stack = new long[32];
            for (int i = 0; i < stack.Length; ++i)
            {
                stack[i] = br.ReadInt64();
            }

            var frames = br.ReadUInt16();
            ev._stack.AddRange(stack.Take(frames));

            return ev;
        }
    }

    public class RemoveAllocationRpcEvent : RpcEvent
    {
        public long Pointer { get; private set; }

        public static RemoveAllocationRpcEvent FromStream(BinaryReader br)
        {
            return new RemoveAllocationRpcEvent()
            {
                Pointer = br.ReadInt64()
            };
        }
    }

    public struct PointerData
    {
        public long Pointer { get; set; }

        public string Function { get; set; }

        public PointerData(long pointer, string function)
        {
            Pointer = pointer;
            Function = function;
        }
    }

    public delegate Task DispatchEventDelegate(RpcEvent ev);

    public class RpcServer : IDisposable
    {
        class Request : IDisposable
        {
            private readonly ManualResetEvent _completed = new ManualResetEvent(false);

            public long Id { get; private set; }

            public byte[] Response { get; set; }

            public int ResponseLength { get; set; }

            public Request(long id)
            {
                Id = id;
            }

            public void Dispose()
            {
                _completed.Dispose();
            }

            public void Complete()
                => _completed.Set();

            public async Task Wait()
            {
                using (var cancellationTokenSource = new CancellationTokenSource())
                {
                    await _completed.WaitOneAsync(cancellationTokenSource.Token);
                }
            }

            public async Task<bool> Wait(TimeSpan timeout)
            {
                using (var cancellationTokenSource = new CancellationTokenSource())
                {
                    return await _completed.WaitOneAsync(timeout, cancellationTokenSource.Token);
                }
            }
        }

        private const string PipeName = "nl-trace-4661A80D-CD1F-4692-9269-BCC420539E38";

        private readonly byte[] _readBuffer = new byte[65536];

        private NamedPipeServerStream _pipeServer = null;
        private long _outstandingIO = 0;

        private List<RpcEvent> _events = new List<RpcEvent>();
        private readonly ReaderWriterLockSlim _eventsLock = new ReaderWriterLockSlim(LockRecursionPolicy.NoRecursion);

        private long _nextRequestId = 0;
        private readonly Dictionary<long, Request> _requests = new Dictionary<long, Request>();
        private readonly ReaderWriterLockSlim _requestsLock = new ReaderWriterLockSlim(LockRecursionPolicy.NoRecursion);

        public event DispatchEventDelegate OnEvent;

        private long _nextPacketIndex = 0;

        public RpcServer()
        {
            StartNewPipeServer();
        }

        public void Dispose()
        {
            if (_pipeServer != null)
            {
                var wasConnected = _pipeServer.IsConnected;

                _pipeServer.Dispose();
                _pipeServer = null;

                if (wasConnected)
                    AddDispatchEvent(new DisconnectedRpcEvent());
            }

            while (_outstandingIO != 0)
                Thread.Sleep(100);
        }

        public async Task DispatchEventsAsync()
        {
            if (OnEvent == null)
                throw new InvalidOperationException("OnEvent handler must be set.");

            List<RpcEvent> events;

            _eventsLock.EnterWriteLock();
            try
            {
                events = _events;
                _events = new List<RpcEvent>();
            }
            finally
            {
                _eventsLock.ExitWriteLock();
            }

            foreach (var ev in events)
            {
                await OnEvent(ev);
                ev.Dispose();
            }
        }

        private void AddDispatchEvent(RpcEvent ev)
        {
            _eventsLock.EnterWriteLock();
            try
            {
                _events.Add(ev);
            }
            finally
            {
                _eventsLock.ExitWriteLock();
            }
        }

        private void StartNewPipeServer()
        {
            if (_pipeServer != null)
                throw new InvalidOperationException("Cannot start a new pipe server before destroying previous instance.");

            if (_outstandingIO != 0)
                throw new InvalidOperationException("Outstanding I/O is not zero.");

            _nextPacketIndex = 0;

            _pipeServer = new NamedPipeServerStream(PipeName, PipeDirection.InOut, 1, PipeTransmissionMode.Message, PipeOptions.Asynchronous);
            Interlocked.Increment(ref _outstandingIO);
            _pipeServer.BeginWaitForConnection(OnConnectionAsync, null);
        }

        private void ReleaseServerReference(bool restart)
        {
            if (Interlocked.Decrement(ref _outstandingIO) == 0)
            {
                if (_pipeServer != null)
                {
                    var wasConnected = _pipeServer.IsConnected;
                    _pipeServer.Dispose();
                    _pipeServer = null;

                    if (wasConnected)
                        AddDispatchEvent(new DisconnectedRpcEvent());
                }

                if (restart)
                    StartNewPipeServer();
            }
        }

        private void OnConnectionAsync(IAsyncResult ar)
        {
            if (_pipeServer == null)
            {
                ReleaseServerReference(false);
                return;
            }

            try
            {
                _pipeServer.EndWaitForConnection(ar);
            }
            catch (Exception ex)
            {
                ReleaseServerReference(false);
                MessageBox.Show(ex.Message, "RpcServer", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            Debug.WriteLine($"New pipe connection");

            AddDispatchEvent(new ConnectedRpcEvent());

            _pipeServer.BeginRead(_readBuffer, 0, _readBuffer.Length, OnReadAsync, null);
        }

        private void OnReadAsync(IAsyncResult ar)
        {
            if (_pipeServer == null)
            {
                ReleaseServerReference(false);
                return;
            }

            int read;
            try
            {
                read = _pipeServer.EndRead(ar);
            }
            catch (Exception ex)
            {
                ReleaseServerReference(false);
                MessageBox.Show(ex.Message, "RpcServer", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            if (read == 0)
            {
                // pipe closed probably...
                Debug.WriteLine($"Pipe closed");
                ReleaseServerReference(true);
                return;
            }

            //Debug.WriteLine($"Read {read} bytes");

            ProcessData(read);

            _pipeServer.BeginRead(_readBuffer, 0, _readBuffer.Length, OnReadAsync, null);
        }

        private void OnWriteAsync(IAsyncResult ar)
        {
            if (_pipeServer == null)
            {
                ReleaseServerReference(false);
                return;
            }

            try
            {
                _pipeServer.EndWrite(ar);
            }
            catch (Exception ex)
            {
                ReleaseServerReference(false);
                MessageBox.Show(ex.Message, "RpcServer", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            ReleaseServerReference(false);
        }

        private void ProcessData(int count)
        {
            using (var ms = new MemoryStream(_readBuffer, 0, count, false))
            using (var br = new BinaryReader(ms, Encoding.UTF8, true))
            {
                var expectedNextPacketIndex = Interlocked.Increment(ref _nextPacketIndex);

                var packet_index = br.ReadInt64();
                if (packet_index != expectedNextPacketIndex)
                {
                    Debug.WriteLine(Util.ToHex(_readBuffer, 0, count));
                    throw new Exception($"Expected packet index wrong: {packet_index} != {expectedNextPacketIndex}");
                }

                var command = br.ReadInt32();
                if (command == 0) // add allocation
                {
                    AddDispatchEvent(AddAllocationRpcEvent.FromStream(br));
                }
                else if (command == 1) // remove allocation
                {
                    AddDispatchEvent(RemoveAllocationRpcEvent.FromStream(br));
                }
                else if (command == 2) // pointer data
                {
                    var requestId = br.ReadInt64();

                    var bufferLength = (int)(br.BaseStream.Length - br.BaseStream.Position);
                    var buffer = new byte[bufferLength];
                    Buffer.BlockCopy(_readBuffer, (int)br.BaseStream.Position, buffer, 0, bufferLength);

                    Request request;
                    _requestsLock.EnterWriteLock();
                    try
                    {
                        if (_requests.TryGetValue(requestId, out request))
                            _requests.Remove(requestId);
                    }
                    finally
                    {
                        _requestsLock.ExitWriteLock();
                    }

                    request.Response = buffer;
                    request.ResponseLength = bufferLength;

                    request.Complete();
                }
            }
        }

        public async Task<PointerData> QueryFunctionPointerData(long pointer)
        {
            if (_pipeServer == null)
                throw new InvalidOperationException("Not connected.");

            var request = new Request(Interlocked.Increment(ref _nextRequestId));

            _requestsLock.EnterWriteLock();
            try
            {
                _requests.Add(request.Id, request);
            }
            finally
            {
                _requestsLock.ExitWriteLock();
            }

            using (var ms = new MemoryStream(4096))
            using (var bw = new BinaryWriter(ms, Encoding.UTF8, true))
            {
                bw.Write(3); // command - request function pointer data
                bw.Write(request.Id);
                bw.Write(pointer);

                Interlocked.Increment(ref _outstandingIO);
                _pipeServer.BeginWrite(ms.GetBuffer(), 0, (int)bw.BaseStream.Length, OnWriteAsync, null);
            }

            await request.Wait();

            using (request)
            {
                using (var ms = new MemoryStream(request.Response, 0, request.ResponseLength, false))
                using (var br = new BinaryReader(ms, Encoding.UTF8, true))
                {
                    br.ReadInt64(); // pointer
                    var functionName = br.BaseStream.ReadNullTerminatedString(256);

                    return new PointerData(pointer, functionName);
                }
            }
        }
    }
}
