namespace NativeLibAllocationTrace
{
    partial class MainForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.lvTraces = new System.Windows.Forms.ListView();
            this.columnHeader1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader2 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader3 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.columnHeader4 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.statusStrip1 = new System.Windows.Forms.StatusStrip();
            this.tslPointerCount = new System.Windows.Forms.ToolStripStatusLabel();
            this.tslMemorySize = new System.Windows.Forms.ToolStripStatusLabel();
            this.tslTotalAllocations = new System.Windows.Forms.ToolStripStatusLabel();
            this.statusStrip1.SuspendLayout();
            this.SuspendLayout();
            // 
            // lvTraces
            // 
            this.lvTraces.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.lvTraces.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1,
            this.columnHeader2,
            this.columnHeader3,
            this.columnHeader4});
            this.lvTraces.Font = new System.Drawing.Font("Consolas", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.lvTraces.FullRowSelect = true;
            this.lvTraces.GridLines = true;
            this.lvTraces.HideSelection = false;
            this.lvTraces.Location = new System.Drawing.Point(12, 12);
            this.lvTraces.Name = "lvTraces";
            this.lvTraces.Size = new System.Drawing.Size(776, 375);
            this.lvTraces.TabIndex = 0;
            this.lvTraces.UseCompatibleStateImageBehavior = false;
            this.lvTraces.View = System.Windows.Forms.View.Details;
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "Time";
            // 
            // columnHeader2
            // 
            this.columnHeader2.Text = "File";
            // 
            // columnHeader3
            // 
            this.columnHeader3.Text = "Function";
            // 
            // columnHeader4
            // 
            this.columnHeader4.Text = "Pointer";
            // 
            // statusStrip1
            // 
            this.statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.tslPointerCount,
            this.tslMemorySize,
            this.tslTotalAllocations});
            this.statusStrip1.Location = new System.Drawing.Point(0, 428);
            this.statusStrip1.Name = "statusStrip1";
            this.statusStrip1.Size = new System.Drawing.Size(800, 22);
            this.statusStrip1.TabIndex = 1;
            this.statusStrip1.Text = "statusStrip1";
            // 
            // tslPointerCount
            // 
            this.tslPointerCount.Name = "tslPointerCount";
            this.tslPointerCount.Size = new System.Drawing.Size(53, 17);
            this.tslPointerCount.Text = "ptrcount";
            // 
            // tslMemorySize
            // 
            this.tslMemorySize.Name = "tslMemorySize";
            this.tslMemorySize.Size = new System.Drawing.Size(54, 17);
            this.tslMemorySize.Text = "memsize";
            // 
            // tslTotalAllocations
            // 
            this.tslTotalAllocations.Name = "tslTotalAllocations";
            this.tslTotalAllocations.Size = new System.Drawing.Size(56, 17);
            this.tslTotalAllocations.Text = "totalalloc";
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(800, 450);
            this.Controls.Add(this.statusStrip1);
            this.Controls.Add(this.lvTraces);
            this.Name = "MainForm";
            this.Text = "NativeLib Allocation Tracer";
            this.statusStrip1.ResumeLayout(false);
            this.statusStrip1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ListView lvTraces;
        private System.Windows.Forms.ColumnHeader columnHeader1;
        private System.Windows.Forms.ColumnHeader columnHeader2;
        private System.Windows.Forms.ColumnHeader columnHeader3;
        private System.Windows.Forms.ColumnHeader columnHeader4;
        private System.Windows.Forms.StatusStrip statusStrip1;
        private System.Windows.Forms.ToolStripStatusLabel tslPointerCount;
        private System.Windows.Forms.ToolStripStatusLabel tslMemorySize;
        private System.Windows.Forms.ToolStripStatusLabel tslTotalAllocations;
    }
}

