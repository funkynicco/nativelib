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
        private readonly RpcServer _rpc = new RpcServer();
        private readonly Timer _timer;
        private readonly Dictionary<long, PointerData> _pointerDataMap = new Dictionary<long, PointerData>();

        public MainForm()
        {
            _rpc.OnEvent += OnRpcEventAsync;

            InitializeComponent();

            Resize += (sender, e) => UpdateSizing();
            UpdateSizing();

            var lvi = new ListViewItem($"162.394");
            lvi.SubItems.Add(@"D:\Coding\CPP\MyLibs\nativelib\tool\NativeLibAllocationTrace\src\test.cpp:359");
            lvi.SubItems.Add("CMyClassName::AllocateOverlapped");
            lvi.SubItems.Add($"0xCCDA23AF38D9040D");
            lvTraces.Items.Add(lvi);

            //throw new Exception("ask for pointer symbolic info");
            _timer = new Timer()
            {
                Interval = 100,
                Enabled = true
            };
            _timer.Tick += _timer_Tick;
        }

        private async void _timer_Tick(object sender, EventArgs e)
        {
            (sender as Timer).Enabled = false;

            await _rpc.DispatchEventsAsync();

            (sender as Timer).Enabled = true;
        }

        private async Task<PointerData> GetPointerDataAsync(long pointer)
        {
            if (!_pointerDataMap.TryGetValue(pointer, out PointerData pointerData))
            {
                pointerData = await _rpc.QueryFunctionPointerData(pointer);
                _pointerDataMap.Add(pointer, pointerData);

                Debug.WriteLine($"Resolved function pointer 0x{pointer.ToString("x8")} ==> {pointerData.Function}");
            }

            return pointerData;
        }

        private async Task OnRpcEventAsync(RpcEvent ev)
        {
            if (ev is ConnectedRpcEvent)
            {
                lvTraces.Items.Clear();
                _pointerDataMap.Clear();
            }
            else if (ev is DisconnectedRpcEvent)
            {
            }
            else if (ev is AddAllocationRpcEvent addAllocationEvent)
            {
                var pointerData = GetPointerDataAsync(addAllocationEvent.Pointer);
                foreach (var ptr in addAllocationEvent.Stack)
                {
                    var info = await GetPointerDataAsync(ptr);
                    // ...
                }

                var lvi = new ListViewItem($"{addAllocationEvent.Time:0.000}");
                lvi.Tag = addAllocationEvent.Pointer;
                lvi.SubItems.Add($"{addAllocationEvent.Filename}:{addAllocationEvent.Line}");
                lvi.SubItems.Add(addAllocationEvent.Function);
                lvi.SubItems.Add($"0x{addAllocationEvent.Pointer.ToString("X8")} ({addAllocationEvent.SizeOfPointerData} B)");
                lvTraces.Items.Add(lvi);
            }
            else if (ev is RemoveAllocationRpcEvent removeAllocationEvent)
            {
                for (int i = 0; i < lvTraces.Items.Count; ++i)
                {
                    var lvi = lvTraces.Items[i];
                    var pointer = (long)lvi.Tag;
                    if (pointer == removeAllocationEvent.Pointer)
                    {
                        lvTraces.Items.RemoveAt(i);
                        break;
                    }
                }
            }
        }

        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                _rpc.Dispose();

                if (components != null)
                    components.Dispose();
            }

            base.Dispose(disposing);
        }

        private void UpdateSizing()
        {
            int width = lvTraces.ClientSize.Width;

            lvTraces.Columns[0].Width = (int)(width * 0.1);
            lvTraces.Columns[1].Width = (int)(width * 0.5);
            lvTraces.Columns[2].Width = (int)(width * 0.2);
            lvTraces.Columns[3].Width = (int)(width * 0.2);
        }
    }
}
