using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace GS_UpdDB
{
    /// <summary>
    /// Логика взаимодействия для Window1.xaml
    /// </summary>
    public class DataItem
    {
        public string Column1 { get; set; }
        public string Column2 { get; set; }
        public string Column3 { get; set; }
        public string Column4 { get; set; }
    }/// 
    public partial class Window1 : Window
    {
        public Window1()
        {
            InitializeComponent();

            // Your programmatically created DataGrid is attached to MainGrid here                       

            var dg = this.MainGrid;

            // create four columns here with same names as the DataItem's properties


            DataGridTextColumn column;
            column = new DataGridTextColumn();
            column.Header = "Column1";
            column.Binding = new Binding("Column1");
            column.Width = 50;
            dg.Columns.Add(column);

            column = new DataGridTextColumn();
            column.Header = "Column2";
            column.Binding = new Binding("Column2");
            column.Width = 50;
            dg.Columns.Add(column);

            column = new DataGridTextColumn();
            column.Header = "Column3";
            column.Binding = new Binding("Column3");
            column.Width = 50;
            dg.Columns.Add(column);

            column = new DataGridTextColumn();
            column.Header = "Column4";
            column.Binding = new Binding("Column4");
            column.Width = 50;
            dg.Columns.Add(column);

            // create and add two lines of fake data to be displayed, here

            for (int i = 1; i <= 20; i++)
            {
                dg.Items.Add(new DataItem { Column1 = "a."+ i, Column2 = "a."+ i+1, Column3 = "a."+ i+2, Column4 = "a."+ i+3 });
             
            }

        }
    }
}
