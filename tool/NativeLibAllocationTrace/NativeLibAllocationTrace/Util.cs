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

        public static string GetSize(long size)
        {
            var types = new string[] { "B", "KB", "MB", "GB", "TB", "PB", "EB" };
            var type = 0;

            var val = (double)size;
            while (val >= 1024.0)
            {
                val /= 1024.0;
                ++type;
            }

            if (type == 0)
                return $"{size} B";

            return $"{val:0.00} {types[type]}";
        }
    }
}
