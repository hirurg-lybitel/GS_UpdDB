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
    public partial class MyMessageBox : Window
    {
        public MyMessageBox()
        {
            InitializeComponent();

        }

        public MyMessageBox(String Message)
        {
            InitializeComponent();

            tbMessage.Text = Message;
        }

        public MyMessageBox(String Message, MessageBoxButton mBut = MessageBoxButton.OKCancel)
        {
            InitializeComponent();

            tbMessage.Text = Message;

            switch (mBut) {
                case MessageBoxButton.OK:
                    btnCancelColumn.Width = new GridLength(0);
                    btnOk.Content = "OK";
                    break;
            }
        }


        private void btnOk_Click(object sender, RoutedEventArgs e)
        {
            DialogResult = true;                

        }


        private void btnCancel_Click(object sender, RoutedEventArgs e)
        {
            DialogResult = false;
        }
    }
}
