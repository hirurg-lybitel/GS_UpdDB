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
    /// Логика взаимодействия для AboutProgram.xaml
    /// </summary>
    public partial class AboutProgram : Window
    {
        public AboutProgram(MainWindow mWind)
        {
            InitializeComponent();

            lbVersion.Content = mWind.CurrentApplicationVersion();
        }

        private void btnOk_Click(object sender, RoutedEventArgs e)
        {
            DialogResult = true;
        }
    }
}
