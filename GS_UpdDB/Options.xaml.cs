using System;
using System.Collections.ObjectModel;
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
using System.IO;

namespace GS_UpdDB
{  
    public partial class Options : Window
    {
        private class Categories
        {
            public int Key { get; set; }
            public Categories Parent { get; set; }
            public String Name { get; set; }
            public StackPanel objPanel { get; set; }
        }

        private const int CategoriesCount = 3;

        private TreeViewItem GetTreeViewItemByKey(ItemCollection itemCol,  int iKey) {
            foreach (TreeViewItem item in itemCol)
            {
                if (item.Items.Count > 0) {
                    TreeViewItem it = GetTreeViewItemByKey(item.Items, iKey);
 
                    if (it != null) return it;                   
                }
                Categories ct = (Categories)item.Tag;

                if (ct.Key == iKey) return item;
            }
            return null;
        }

        public bool isSearchGedeminAfterOpen;
        public bool isLogging;
        public String sLogPath;
        public String sCurrentDir;
        public String sSettingFileName;
        public String sGedeminPath;
        public String sLastDBPath;
        public String sLastNSPath;
        public String sNSPath;

        private void InitializeCategories() {
            Categories[] arrCategories = new Categories[CategoriesCount];

            arrCategories[0] = new Categories { Key = 1, Parent = null, Name = "Общие", objPanel = null };
            arrCategories[1] = new Categories { Key = 2, Parent = arrCategories[0], Name = "Сохранённые пути", objPanel = (StackPanel)FindName("StackPanel_1") };
            arrCategories[2] = new Categories { Key = 3, Parent = arrCategories[0], Name = "Логирование", objPanel = (StackPanel)FindName("StackPanel_2") };
            

            foreach (Categories ct in arrCategories)
            {
                TreeViewItem item = new TreeViewItem();
                item.Tag = ct;
                item.Header = ct.Name;

                if (ct.Parent != null)
                {
                    GetTreeViewItemByKey(CategoryView.Items, ct.Parent.Key).Items.Add(item);
                    continue;
                }
                CategoryView.Items.Add(item);
                
            }
        }

        private void LoadParams(MainWindow mWind) {
            tbSettingsPath.Text         = mWind.sSettingFileName;
            tbGedeminPath.Text          = mWind.sGedeminPath;
            tbLastDBPath.Text           = mWind.sLastDBPath;
            tbLastNSPath.Text           = mWind.sLastNSPath;
            cbSearchGedemin.IsChecked   = mWind.isSearchGedeminAfterOpen;
            cbIsLogging.IsChecked       = mWind.isLogging;
            tbLogPath.Text              = mWind.sLogPath;
        }

        public Options(MainWindow mWind)
        {
            InitializeComponent();

            InitializeCategories();

            LoadParams(mWind);

        }

        private void CategoryView_SelectedItemChanged(object sender, RoutedPropertyChangedEventArgs<object> e)
        {
            TreeViewItem value;
            Categories ct;

            value = (TreeViewItem)e.OldValue;
            if (value != null) {
                ct = (Categories)(value.Tag);
                if (ct.objPanel !=null)
                    ct.objPanel.Visibility = Visibility.Hidden;
            }

            value = (TreeViewItem)e.NewValue;
            if (value != null)
            {
                ct = (Categories)(value.Tag);
                if (ct.objPanel != null)
                    ct.objPanel.Visibility = Visibility.Visible;
            }

        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            var a = CategoryView;

            var root = new ObservableCollection<Categories>{
                new Categories {
                    Key = 1,
                    Name = "gfd"
                }
            };

            CategoryView.ItemsSource = root;
            //Title = "Explorer View (" + Path.GetFullPath(folder) + ")";            

        }

        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            isSearchGedeminAfterOpen    = (bool)cbSearchGedemin.IsChecked;
            isLogging                   = (bool)cbIsLogging.IsChecked;
            sLogPath                    = tbLogPath.Text;
            sGedeminPath                = tbGedeminPath.Text;
            sLastDBPath                 = tbLastDBPath.Text;
            sLastNSPath                 = tbLastNSPath.Text;
        }

        private void btnOk_Click(object sender, RoutedEventArgs e) {
            DialogResult = true;
        }
    }
}
