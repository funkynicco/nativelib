using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.IO.Pipes;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace NativeLibAllocationTrace
{
    public partial class MainForm : Form
    {
        public MainForm()
        {
            InitializeComponent();

            Shown += MainForm_Shown;
        }

        private string ReadNullTerminatedString(Stream stream, int sizeOfBuffer)
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

        private async void MainForm_Shown(object sender, EventArgs e)
        {
            while (true)
            {
                using (var stream = new NamedPipeServerStream("nl-trace-4661A80D-CD1F-4692-9269-BCC420539E38", PipeDirection.InOut, 1))
                {
                    await stream.WaitForConnectionAsync();

                    Debug.WriteLine($"Connection accepted");

                    var buffer = new byte[65536];

                    while (true)
                    {
                        var read = await stream.ReadAsync(buffer, 0, buffer.Length);
                        if (read == 0)
                            break;

                        Debug.WriteLine($"read {read} bytes");

                        var ms = new MemoryStream(buffer, 0, read, false);
                        var br = new BinaryReader(ms);

                        var command = br.ReadInt32();
                        if (command == 0) // add allocation
                        {
                            var time = br.ReadDouble();
                            var filename = ReadNullTerminatedString(br.BaseStream, 260);
                            int line = br.ReadInt32();
                            var function = ReadNullTerminatedString(br.BaseStream, 64);
                            var pointer = br.ReadInt64();

                            var lvi = new ListViewItem($"{time:0.000}");
                            lvi.SubItems.Add($"{filename}:{line}");
                            lvi.SubItems.Add(function);
                            lvi.SubItems.Add($"0x{pointer.ToString("X8")}");
                            lvTraces.Items.Add(lvi);
                        }
                        else if (command == 1) // remove allocation
                        {

                        }
                    }
                }
            }
        }
    }
}
