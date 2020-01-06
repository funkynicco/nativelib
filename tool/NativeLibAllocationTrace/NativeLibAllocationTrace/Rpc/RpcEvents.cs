using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NativeLibAllocationTrace.Rpc
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
}
