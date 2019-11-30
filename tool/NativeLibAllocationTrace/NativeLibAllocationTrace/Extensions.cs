using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace NativeLibAllocationTrace
{
    public static class Extensions
    {
        public static void WriteNullTerminatedString(this Stream stream, string value, int sizeOfBuffer)
        {
            int byteCount = Encoding.UTF8.GetByteCount(value);
            if (byteCount + 1 > sizeOfBuffer)
                throw new ArgumentOutOfRangeException("The buffer is too small for the value.");

            var bytes = new byte[sizeOfBuffer];
            Encoding.UTF8.GetBytes(value, 0, value.Length, bytes, 0);
            bytes[bytes.Length - 1] = 0;
            stream.Write(bytes, 0, bytes.Length);
        }

        public static string ReadNullTerminatedString(this Stream stream, int sizeOfBuffer)
        {
            var bytes = new byte[sizeOfBuffer];
            stream.Read(bytes, 0, bytes.Length);

            for (int i = 0; i < bytes.Length; ++i)
            {
                if (bytes[i] == 0)
                    return Encoding.UTF8.GetString(bytes, 0, i);
            }

            throw new Exception("Invalid buffers string");
        }

        public static async Task<bool> WaitOneAsync(this WaitHandle handle, int millisecondsTimeout, CancellationToken cancellationToken)
        {
            RegisteredWaitHandle registeredHandle = null;
            CancellationTokenRegistration tokenRegistration = default;
            try
            {
                var source = new TaskCompletionSource<bool>();
                registeredHandle = ThreadPool.RegisterWaitForSingleObject(
                    handle,
                    (state, timedOut) => ((TaskCompletionSource<bool>)state).TrySetResult(!timedOut),
                    source,
                    millisecondsTimeout,
                    true);

                tokenRegistration = cancellationToken.Register(
                    state => ((TaskCompletionSource<bool>)state).TrySetCanceled(),
                    source);

                return await source.Task;
            }
            finally
            {
                if (registeredHandle != null)
                    registeredHandle.Unregister(null);

                tokenRegistration.Dispose();
            }
        }

        public static Task<bool> WaitOneAsync(this WaitHandle handle, TimeSpan timeout, CancellationToken cancellationToken)
        {
            return handle.WaitOneAsync((int)timeout.TotalMilliseconds, cancellationToken);
        }

        public static Task<bool> WaitOneAsync(this WaitHandle handle, CancellationToken cancellationToken)
        {
            return handle.WaitOneAsync(Timeout.Infinite, cancellationToken);
        }
    }
}
