using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NativeLibAllocationTrace
{
    public static class Util
    {
        public static string ToHex(byte[] data, int offset, int length)
        {
            var sb = new StringBuilder(length * 2);

            for (int i = 0; i < length; ++i)
            {
                var b = data[offset + i];
                sb.Append(b.ToString("x2"));
            }

            return sb.ToString();
        }
    }
}
