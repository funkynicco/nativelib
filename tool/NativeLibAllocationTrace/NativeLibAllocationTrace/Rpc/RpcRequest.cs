using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace NativeLibAllocationTrace.Rpc
{
    public partial class RpcServer
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
    }
}