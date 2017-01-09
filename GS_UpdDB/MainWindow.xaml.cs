using System;
using System.Collections;
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
using System.Windows.Navigation;
using System.Windows.Shapes;
using Microsoft.Win32;
using System.Drawing;
using System.ComponentModel;

using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Security.Cryptography;
using System.Threading;
using System.Diagnostics;
using m_Forms = System.Windows.Forms;
using dlgResult = System.Windows.Forms.DialogResult;

using DWORD = System.UInt32;
using Path = System.IO.Path;

using System.Net;
using System.Globalization;

namespace GS_UpdDB
{
    public partial class MainWindow : Window {

        /******************Пользовательские свойсва компонентов******************/

        public static readonly DependencyProperty CustomValueProperty = DependencyProperty.RegisterAttached(
            "HideComponent", typeof(Boolean), typeof(MainWindow), new PropertyMetadata(false));

        public static void SetHideComponent(DependencyObject element, Boolean value)
        {
            element.SetValue(CustomValueProperty, value);
        }

        public static Boolean GetHideComponent(DependencyObject element)
        {
            return (bool)element.GetValue(CustomValueProperty);
        }

        public Boolean HideComponent
        {
            get
            {
                return (Boolean)GetValue(CustomValueProperty);
            }
            set
            {
                SetValue(CustomValueProperty, value);
            }
        }

        /************************************************************************/

        private class m_DataBase
        {
            public int Number { get; set; }
            public String DBPath { get; set; }
        }

        private class m_NameSpace
        {
            public int Number { get; set; }
            public String NSPath { get; set; }
            public String NSFileName { get; set; }
        }

        /*******************************Переменные*******************************/
        DWORD result;
        MyMessageBox mes;

        private OpenFileDialog ofdGedeminPath;
        private OpenFileDialog ofdNameSpace;
        private OpenFileDialog ofdDBPath;
        private m_Forms.FolderBrowserDialog fbdNameSpaceFolder;

        public bool isSearchGedeminAfterOpen;
        public bool isLogging;
        public String sLogPath;
        public String sCurrentDir;
        public String sSettingFileName;
        public String sGedeminPath;
        public String sLastDBPath;
        public String sLastNSPath;
        public String sNSPath;
        private String searchDBPath;
        private String searchNSPath;
        private String searchGedeminAfterOpen;
        private String searchGedeminPath;
        private String searchLogPath;
        private String searchIsLogging;


        private bool isPackage;
        private const int MAX_PATH = 260;
        private bool isApplicationExitCall = false;
        private bool IsInProcess;

        private const String g_urlNewFile = "https://github.com/hirurg-lybitel/GS_UpdDB/blob/master/GS_UpdDB/bin/Release/GS_UpdDB.exe";
        private const String g_NewFileName = "GS_UpdDB.update";
        
        private const String g_urlUpdater = "https://github.com/hirurg-lybitel/Updater/blob/master/Updater/bin/Release/Updater.exe";
        private const String g_UpdaterFileName= "updater.exe";

        /************************************************************************/

        public MainWindow() {
            InitializeComponent();

            InitializeGlobalVariables();

            Application.Current.MainWindow.Closing += new CancelEventHandler(MainWin_Closing);
        }

        private void InitializeGlobalVariables() {
            ofdGedeminPath = new OpenFileDialog();
            ofdNameSpace = new OpenFileDialog();
            ofdDBPath = new OpenFileDialog();
            fbdNameSpaceFolder = new m_Forms.FolderBrowserDialog();

            searchGedeminPath = "GedeminPath";
            searchDBPath = "LastDBPath";
            searchNSPath = "LastNSPath";
            searchGedeminAfterOpen = "SearchGedeminAfterOpen";
            searchGedeminPath = "GedeminPath";
            searchLogPath = "LogPath";
            searchIsLogging = "IsLogging";
            sGedeminPath = String.Empty;
            sLastDBPath = String.Empty;
            sLastNSPath = String.Empty;
            sNSPath = String.Empty;
            sLogPath = String.Empty;

            IsInProcess = false;
            isLogging = true;
            isSearchGedeminAfterOpen = false;


            /*ItemSource для dgvNameSpace*/
            dgNameSpaceItemsSource = new System.Collections.ObjectModel.ObservableCollection<m_NameSpace>();

            dgvNameSpace.SetBinding(ItemsControl.ItemsSourceProperty, new Binding
            {
                Source = dgNameSpaceItemsSource
            });

            /*ItemSource для dgvDBPath*/
            dgDataBaseItemsSource = new System.Collections.ObjectModel.ObservableCollection<m_DataBase>();

            dgvDBPath.SetBinding(ItemsControl.ItemsSourceProperty, new Binding
            {
                Source = dgDataBaseItemsSource
            });
            
        }




        /****************************WinAPI functions****************************/
        [DllImport("user32")]
        static extern uint GetWindowThreadProcessId(IntPtr hWnd, IntPtr ProcessId);

        /*Запись данных из settings.ini*/
        [DllImport("kernel32")]
        private static extern DWORD WritePrivateProfileString(String section, String key, String val, String filePath);

        [DllImport("kernel32")]
        private static extern DWORD GetLastError();

        /*Чтение данных из settings.ini*/
        [DllImport("kernel32", CharSet = CharSet.Unicode)]
        private static extern int GetPrivateProfileString(String section, String key, String def, IntPtr retVal, int size, String filePath);

        /************************************************************************/






        /************************Вспомогательные функции*************************/
        private bool EnumWindowsProcMy(IntPtr hwnd, IntPtr lParam)
        {
            IntPtr g_HWND;
            IntPtr lpdwProcessId = IntPtr.Zero;

            GetWindowThreadProcessId(hwnd, lpdwProcessId);
            if (lpdwProcessId == lParam)
            {
                g_HWND = hwnd;
                return false;
            }
            return true;
        }

        private bool DirectoryExists(String dir = "") {
            bool res = false;
            if (Directory.Exists(dir)) res = true;

            return res;
        }

        private bool DeleteDirectory(String sDir, bool bDeleteSubDir = true) {

            DirectoryInfo di = new DirectoryInfo(sDir);
            try
            {
                foreach (FileInfo file in di.GetFiles())
                {
                    file.Delete();
                }
                foreach (DirectoryInfo dir in di.GetDirectories())
                {
                    dir.Delete(bDeleteSubDir);
                }

                Directory.Delete(sDir);
            }
            catch
            {
                return false;
            }

            return true;
        }

        private String GetMD5(String str)
        {
            MD5 md5 = System.Security.Cryptography.MD5.Create();

            byte[] inputBytes = (new System.Text.UTF8Encoding()).GetBytes(str);

            byte[] hash = md5.ComputeHash(inputBytes);

            StringBuilder sb = new StringBuilder();

            for (int i = 0; i < hash.Length; i++)
            {
                sb.Append(hash[i].ToString("X2"));
//                md5.TransformBlock(hash, 0, i, hash, 0);
            }
            //md5.TransformFinalBlock(hash, 0, 0);

            return sb.ToString();
        }

        private String GetRUIDNSFile(String FileName)
        {
            String search = "RUID";
            String result = String.Empty;
            String line;

            StreamReader fileInput = new StreamReader(FileName);

            int pos;
            while ((line = fileInput.ReadLine()) != null){
                pos = line.IndexOf(search);
                if (pos > 0) {
                    result = line.Substring(pos + 6, line.Length - (pos + 6));
                    break;
                }
            }
            return result;
        }

        private bool copyFile(String cFrom, String cTo)
        {
            File.Copy(cFrom, cTo, true);
            return true;
        }

        private void RecursiveSearch(String sPath, String sSearch, bool OnlyFirst)
        {
            if (OnlyFirst && sGedeminPath != String.Empty) return;

            Dispatcher.BeginInvoke((Action)(() => this.tbGedeminPath.Text = sPath));

            foreach (string file in Directory.GetFiles(sPath, sSearch))
            {
                sGedeminPath = sPath;
            }
            foreach (string subdir in Directory.GetDirectories(sPath))
            {
                try
                {
                    RecursiveSearch(subdir, sSearch, OnlyFirst);
                }
                catch { }
            }
        }

        private void SearchOnAllDrives(String sSearch, bool OnlyFirst = true)
        {
            foreach (DriveInfo d in DriveInfo.GetDrives().Where(x => x.IsReady))
            {
                RecursiveSearch(d.Name, sSearch, OnlyFirst);
                if (OnlyFirst && sGedeminPath != String.Empty) return;
            }
        }

        private void MyThread(String sCmdLine)
        {
            //Dispatcher.BeginInvoke((Action)(() => this.tbGedeminPath.Text = sGedeminPath));
            //Dispatcher.BeginInvoke((Action)(() => this.tbGedeminPath.Background = Brushes.PaleGreen));

            Process cmd = new Process();
            cmd.StartInfo.WindowStyle = ProcessWindowStyle.Hidden;
            cmd.StartInfo.CreateNoWindow = true;
            cmd.StartInfo.UseShellExecute = false;
            cmd.StartInfo.FileName = "cmd.exe";
            cmd.StartInfo.Arguments = "/c " + sCmdLine;

            cmd.Start();
            cmd.WaitForExit();

            if (cmd.ExitCode > 0)
                AddToEventLog("Неверная команда: \n" + sCmdLine, true, false);
        }

        private void MainThreadEvent(Thread[] ThreadArr) {
            for (int i = 0; i < ThreadArr.Length; i++)
            {
                ThreadArr[i].Join();
            }
            IsInProcess = false;

            AddToEventLog("Обновление завершено", false, true);

        }

        private String GetLastErrorDescription(DWORD errCode = 0){
            String errorMessage = new Win32Exception(Marshal.GetLastWin32Error()).Message;

            return errorMessage;
        }

        /*Чтение данных из settings.ini*/
        public DWORD _GetPrivateProfileString(String section, String key, String def, out String retVal, int size, string filePath)
        {
            IntPtr pMem = Marshal.AllocHGlobal(4096 * sizeof(char));
            String temp = "";

            int count = GetPrivateProfileString(section, key, null, pMem, 4096 * sizeof(char), sSettingFileName) - 1;

            if (count > 0)
            {
                temp = Marshal.PtrToStringUni(pMem, count + 1);
            }

            retVal = temp.Split('\0')[0];

            Marshal.FreeHGlobal(pMem);

            if (count < 0) count = 0;

            return (DWORD)count;
        }

        /*Добавление информации в лог*/
        private void AddToEventLog(String str, bool isError, bool withMsgBox)
        {
            if (!(isLogging)) return;

            if (sLogPath == String.Empty) sLogPath = sCurrentDir + "\\log.txt";

            if (!File.Exists(sLogPath)) File.CreateText(sLogPath);

            StreamWriter sw = File.AppendText(sLogPath);
            sw.WriteLine(DateTime.Now.ToString() + " : " + str + ";");
            sw.Close();

            if (withMsgBox)
                (new MyMessageBox(str, MessageBoxButton.OK)).ShowDialog();

        }

        private IEnumerable<System.Windows.Controls.DataGridRow> GetDataGridRows(System.Windows.Controls.DataGrid grid)
        {
            var itemsSource = grid.ItemsSource as System.Collections.IEnumerable;
            if (null == itemsSource) yield return null;
            foreach (var item in itemsSource)
            {
                var row = grid.ItemContainerGenerator.ContainerFromItem(item) as System.Windows.Controls.DataGridRow;
                if (null != row) yield return row;
            }
        }

        /*ItemSource для DataGrid*/
        private System.Collections.ObjectModel.ObservableCollection<m_NameSpace> dgNameSpaceItemsSource { get; set; }

        private System.Collections.ObjectModel.ObservableCollection<m_DataBase> dgDataBaseItemsSource { get; set; }

        private Stack aRows(DataGrid grid){
            Stack myStack = new Stack();

            var rows = GetDataGridRows(grid);

            foreach (DataGridRow r in rows)
            {
                if (null == r) break;
                myStack.Push(r.Item);
            }
            return myStack;
        }

        private m_NameSpace[] aRowsNameSpace() {
            Stack myStack = aRows(dgvNameSpace);
            m_NameSpace[] ns = new m_NameSpace[myStack.Count];

            int i = 0;
            foreach (m_NameSpace ms in myStack)
            {
                ns[i++] = ms;
            }

            return ns;
        }

        private m_DataBase[] aRowsDataBase() {
            Stack myStack = aRows(dgvDBPath);
            m_DataBase[] db = new m_DataBase[myStack.Count];

            int i = 0;
            foreach (m_DataBase ms in myStack)
            {
                db[i++] = ms;
            }

            return db;
        }

        private bool CheckProcess() {
            if (IsInProcess)
            {
                AddToEventLog("Процесс обновления уже запущен!", true, true);
                return true;
            }
            return false;
        }

        private  void DownloadProgress(object sender, DownloadProgressChangedEventArgs e)
        {
            // Displays the operation identifier, and the transfer progress.

            //String s = String.Format("{0}    downloaded {1} of {2} bytes. {3} % complete...",
            //    (string)e.UserState,
            //    e.BytesReceived,
            //    e.TotalBytesToReceive,
            //    e.ProgressPercentage);

            //AddToEventLog(s, false, false);

       //     Thread.Sleep(300);

            AddStatus(String.Format("Скачано {0} байт", e.BytesReceived));        
        }

        private void DownloadCompleted(object sender, AsyncCompletedEventArgs e) {

            Thread.Sleep(300);            

            String mes = "Скачивание завершено";

            Exception err = e.Error;
            if (err != null) {
                mes = err.Message;

                Exception ierr = err.InnerException;
                if (ierr != null) mes = mes + "\n" +  ierr.Message;
            }

            AddStatus(mes); 
            // (new MyMessageBox("Скачивание завершено", MessageBoxButton.OK)).ShowDialog();
        }

        private void DownloadThreadFunction(String url, String FileName) {
            AddToEventLog("Скачивание файла " + FileName, false, false);

            try
            {
                using (var client = new WebClient())
                {

                    // client.DownloadFile(new Uri(url), FileName);

                    client.DownloadProgressChanged += new DownloadProgressChangedEventHandler(DownloadProgress);
                    client.DownloadFileCompleted += new AsyncCompletedEventHandler(DownloadCompleted);

                    
                    client.DownloadFileAsync(new Uri(url), FileName);

                    AddToEventLog("Скачивание файла завершено.", false, false);
                    return ;
                }
            }
            catch (Exception error)
            {
                AddToEventLog("Не удалось скачать файл.\n" + error, true, false);
                AddStatus("Не удалось скачать файл");
                return ;
            }
        }

        private void DownloadFile(String url, String FileName) {

            Thread DownloadThread = new Thread(() => DownloadThreadFunction(url, FileName));
            //DownloadThread.IsBackground = true;
            DownloadThread.Start();

            DownloadThread.Join();

        }

        private void CheckUpdates(bool isSilent) {

            if (!(File.Exists(g_NewFileName))) return;

            try {
                Version versionNewFile      = new Version(FileVersionInfo.GetVersionInfo(g_NewFileName).FileVersion);
                Version versionCurrentFile  = new Version(CurrentApplicationVersion());

                if (versionNewFile > versionCurrentFile) {
                    AddToEventLog("Доступна новая версия программы.", false, false);                    

                    mes = new MyMessageBox("Доступна новая версия программы.\nОбновить сейчас?");
                    if (mes.ShowDialog() == true)
                    {
                        DownloadFile("", g_UpdaterFileName);
                        if (!(File.Exists(g_NewFileName))) return;

                        Process.Start(g_UpdaterFileName, g_NewFileName + " \"" + Process.GetCurrentProcess().ProcessName + "\"");
                        Process.GetCurrentProcess().CloseMainWindow();
                    }
                }
                if (versionNewFile == versionCurrentFile) {
                    AddToEventLog("У вас установлена самая последняя весрия.", false, isSilent);
                    return;
                }
            }
            catch
            {
                AddToEventLog("Ошибка обновления программы.", true, isSilent);
                if (File.Exists(g_NewFileName))  File.Delete(g_NewFileName); 
                if (File.Exists(g_UpdaterFileName)) File.Delete(g_UpdaterFileName);
            }           
        }

        public String CurrentApplicationVersion() {
            FileVersionInfo myFileVersionInfo;
            System.Reflection.Assembly assembly = System.Reflection.Assembly.GetExecutingAssembly();
            myFileVersionInfo = FileVersionInfo.GetVersionInfo(assembly.Location);
            return myFileVersionInfo.FileVersion;
        }

        private void DeleteUpdater() {
            if (!(File.Exists(g_UpdaterFileName))) return;
            try
            {
                File.Delete(g_UpdaterFileName);
            }
            catch(Exception err) {
                AddToEventLog("Ошибка удаления файла " + g_UpdaterFileName + "\n" + err, true, false);
            }
        }

        private void AddStatus(String str) {

          //  sbStatus.Visibility = Visibility.Collapsed;
            Dispatcher.BeginInvoke((Action)(() => this.sbStatus.Visibility = Visibility.Collapsed));

            if (str == String.Empty) return;

           // sbStatus.Visibility = Visibility.Visible;
            Dispatcher.BeginInvoke((Action)(() => this.sbStatus.Visibility = Visibility.Visible));


           // tbStatus.Text = str;
            Dispatcher.BeginInvoke((Action)(() => this.tbStatus.Text = str));

        }
        /************************************************************************/






        /*******************Обработчики событий кнопок и формы*******************/
        private void MainWin_Closing(object sender, CancelEventArgs e){

            if (CheckProcess()) {
                e.Cancel = true;
                return;
            }

            if (e.Cancel) return;

            if (!(this.isApplicationExitCall))
            {
                mes = new MyMessageBox("Вы точно хотите выйти?");                
                if (mes.ShowDialog() == true)
                {
                    isApplicationExitCall = true;
                    Application.Current.Shutdown();
                }
                else
                    e.Cancel = true;
            }
        }

        private void MainWin_Loaded(object sender, RoutedEventArgs e){      

            DeleteUpdater();

            sCurrentDir = Directory.GetCurrentDirectory();

            sSettingFileName = sCurrentDir + "\\Settings.ini";

            string buffer = "";

            result = _GetPrivateProfileString(
                "PathSection",
                searchGedeminPath,
                null,
                out buffer,
                MAX_PATH,
                sSettingFileName);
            if (result > 0)
                sGedeminPath = buffer;
            else
                AddToEventLog("Ошибка чтения параметра " + searchGedeminPath + " в Settings.ini\n" + GetLastErrorDescription(), true, false);

            if (sGedeminPath != String.Empty) {
                tbGedeminPath.Text = sGedeminPath;
                tbGedeminPath.Background = Brushes.PaleGreen;
            }

            result = _GetPrivateProfileString(
                "PathSection",
                searchDBPath,
                null,
                out buffer,
                MAX_PATH,
                sSettingFileName);
            if (result > 0)
                sLastDBPath = buffer;
            else
                AddToEventLog("Ошибка чтения параметра " + searchDBPath + " в Settings.ini\n" + GetLastErrorDescription(), true, false);

            result = _GetPrivateProfileString(
                            "PathSection",
                            searchNSPath,
                            null,
                            out buffer,
                            MAX_PATH,
                            sSettingFileName);
            if (result > 0)
                sLastNSPath = buffer;
            else
                AddToEventLog("Ошибка чтения параметра " + searchNSPath + " в Settings.ini\n" + GetLastErrorDescription(), true, false);

            result = _GetPrivateProfileString(
                            "PathSection",
                            searchLogPath,
                            null,
                            out buffer,
                            MAX_PATH,
                            sSettingFileName);
            if (result > 0)
                sLogPath = buffer;
            else
                AddToEventLog("Ошибка чтения параметра " + searchLogPath + " в Settings.ini\n" + GetLastErrorDescription(), true, false);

            result = _GetPrivateProfileString(
                            "Options",
                            searchGedeminAfterOpen,
                            null,
                            out buffer,
                            MAX_PATH,
                            sSettingFileName);
            if (result > 0)
                isSearchGedeminAfterOpen = (buffer == "1");
            else
                AddToEventLog("Ошибка чтения параметра " + searchGedeminAfterOpen + " в Settings.ini\n" + GetLastErrorDescription(), true, false);

            result = _GetPrivateProfileString(
                            "Options",
                            searchIsLogging,
                            null,
                            out buffer,
                            MAX_PATH,
                            sSettingFileName);
            if (result > 0)
                isLogging = (buffer == "1");
            else
                AddToEventLog("Ошибка чтения параметра " + searchIsLogging + " в Settings.ini\n" + GetLastErrorDescription(), true, false);



        }

        private void btnExite_Click(object sender, EventArgs e){
            if (CheckProcess()) return;

            mes = new MyMessageBox("Вы точно хотите выйти?");
 
            isApplicationExitCall = true;
            if (mes.ShowDialog() == true)
                Application.Current.Shutdown();
        }

        private void btnChooseEXE_Click(object sender, RoutedEventArgs e){

            if (CheckProcess()) return;

            ofdGedeminPath.Title = "Выберите файл gedemin.exe";
            ofdGedeminPath.Filter = "Файлы EXE|*.exe";
            ofdGedeminPath.FileName = "gedemin.exe";
            ofdGedeminPath.RestoreDirectory = true;



            string sGedeminPath;
            if (ofdGedeminPath.ShowDialog() == true)

            {
                sGedeminPath = ofdGedeminPath.FileName;

                tbGedeminPath.Text = sGedeminPath;
                tbGedeminPath.Background = Brushes.PaleGreen;

                result = WritePrivateProfileString(
                    "PathSection",
                    searchGedeminPath,
                    sGedeminPath,
                    sSettingFileName);

                if (result == 0)
                {
                    AddToEventLog("Ошибка при записи в Settings.ini. " + GetLastErrorDescription(), true, false);
                }
            }
        }

        private void btnDBPathAdd_Click(object sender, RoutedEventArgs e){

            if (CheckProcess()) return;

            ofdDBPath.Title = "Выберите базу данных";
            ofdDBPath.Filter = "Файлы базы данных (*.FDB)|*.FDB|Все файлы (*.*)|*.*";
            ofdDBPath.FileName = "";
            ofdDBPath.InitialDirectory = sLastDBPath;


            if (ofdDBPath.ShowDialog() == true) {

                sLastDBPath = Path.GetDirectoryName(ofdDBPath.FileName);

                result = WritePrivateProfileString(
                    "PathSection",
                    searchDBPath,
                    sLastDBPath,
                    sSettingFileName);

                if (result == 0){
                    AddToEventLog("Ошибка при записи в Settings.ini. " + GetLastErrorDescription(), true, false);
                }

                m_DataBase db = new m_DataBase()
                {
                    Number = dgvDBPath.Items.Count,
                    DBPath = ofdDBPath.FileName
                };

                dgDataBaseItemsSource.Add(db);

            }
        }

        private void btnDBPathDelete_Click(object sender, RoutedEventArgs e) {

            if (CheckProcess()) return;

            int iRecCount = dgvDBPath.Items.Count;
            if (iRecCount == 0) return;

            int iCurRow = dgvDBPath.SelectedItems.Count;
            if (iCurRow <= 0) return;

            m_DataBase[] ItemSource = new m_DataBase[iCurRow];

            for (int i = 0; i < dgvDBPath.SelectedItems.Count; i++)
                ItemSource[i] = (m_DataBase)dgvDBPath.SelectedItems[i];

            foreach (m_DataBase db in ItemSource)
            {
                dgDataBaseItemsSource.Remove(db);
            }

                AddToEventLog("Ошибка при удалении записи из списка выбранных БД. " + GetLastErrorDescription(), true, false);
        }

        private void btnNameSpaceAdd_Click(object sender, RoutedEventArgs e) {

            if (CheckProcess()) return;

            ofdNameSpace.Title = "Выберите файлы ПИ";
            ofdNameSpace.Filter = "Файлы YML|*.yml";
            ofdNameSpace.FileName = "";
            ofdNameSpace.Multiselect = true;

            if (sNSPath != String.Empty)
                ofdNameSpace.InitialDirectory = sNSPath;
            else
		        ofdNameSpace.InitialDirectory = sLastNSPath;

            AddToEventLog("Выбор файлов ПИ.", false, false);

            if (ofdNameSpace.ShowDialog() == true){

                sLastNSPath = Path.GetDirectoryName(ofdNameSpace.FileName);

                result = WritePrivateProfileString(
                    "PathSection",
                    searchNSPath,
                    sLastNSPath,
                    sSettingFileName);

                if (result == 0)
                {
                    AddToEventLog("Ошибка при записи в Settings.ini. " + GetLastErrorDescription(), true, false);
                }

                int iFileCount = ofdNameSpace.SafeFileNames.Length;

                for (int i = 0; i < iFileCount; i++){

                    m_NameSpace ns = new m_NameSpace() {
                        Number = dgvNameSpace.Items.Count,
                        NSFileName = ofdNameSpace.SafeFileNames[i],
                        NSPath = ofdNameSpace.FileNames[i]
                    };

                    dgNameSpaceItemsSource.Add(ns);

                }
            }
        }

        private void btnNameSpaceDelete_Click(object sender, RoutedEventArgs e) {

            if (CheckProcess()) return;

            int iRecCount = dgvNameSpace.Items.Count;
            if (iRecCount == 0) return;

            int iCurRow = dgvNameSpace.SelectedItems.Count;
            if (iCurRow <= 0) return;

            m_NameSpace[] ItemSource = new m_NameSpace[iCurRow];

            for (int i = 0; i < dgvNameSpace.SelectedItems.Count; i++)
                ItemSource[i] = (m_NameSpace)dgvNameSpace.SelectedItems[i];

            foreach (m_NameSpace ns in ItemSource) {
                dgNameSpaceItemsSource.Remove(ns);
            }

            AddToEventLog("Ошибка при удалении записи из списка выбранных ПИ. " + GetLastErrorDescription(), true, false);
        }

        private void btnChooseNSPath_Click(object sender, RoutedEventArgs e) {

            if (CheckProcess()) return;

            AddToEventLog("Выбор папки ПИ.", false, false);

            if (fbdNameSpaceFolder.ShowDialog() == dlgResult.OK){
                tbNSPath.Text = fbdNameSpaceFolder.SelectedPath;

                sNSPath = fbdNameSpaceFolder.SelectedPath;
            }
        }

        private void cbIsPackage_CheckedChanged(object sender, RoutedEventArgs e) {

            if (CheckProcess()) return;

            isPackage = (bool)cbIsPackage.IsChecked;

            if (isPackage){
                tbNSPath.Visibility = Visibility.Visible;
                btnChooseNSPath.Visibility = Visibility.Visible;
            }
            else {
                tbNSPath.Visibility = Visibility.Hidden;
                btnChooseNSPath.Visibility = Visibility.Hidden;
            }

            if (!isPackage){
                sNSPath = String.Empty;
                tbNSPath.Text = String.Empty;
            }

        }

        private void btnOptions_Click(object sender, RoutedEventArgs e) {

            Options opt = new Options(this);

            if (opt.ShowDialog() == true)
            {
                isLogging                   = opt.isLogging;
                isSearchGedeminAfterOpen    = opt.isSearchGedeminAfterOpen;

                result = WritePrivateProfileString(
                    "Options",
                    searchIsLogging,
                    (isLogging ? "1" : "0"),
                    sSettingFileName);

                if (result == 0) AddToEventLog("Ошибка при записи в Settings.ini. " + GetLastErrorDescription(), true, false);

                result = WritePrivateProfileString(
                    "Options",
                    searchGedeminAfterOpen,
                    (isSearchGedeminAfterOpen ? "1" : "0"),
                    sSettingFileName);

                if (result == 0) AddToEventLog("Ошибка при записи в Settings.ini. " + GetLastErrorDescription(), true, false);

            }
            else {
            }

        }

        private void btnAbout_Click(object sender, RoutedEventArgs e) {

            AboutProgram about = new AboutProgram(this);

            about.ShowDialog();
        }

        private void btnUpdate_Click(object sender, RoutedEventArgs e) {

            DownloadFile(g_urlNewFile, g_NewFileName);          

            return;

            CheckUpdates(true);

            DeleteUpdater();

        }

        private void btnRun_Click(object sender, RoutedEventArgs e) {

            if (CheckProcess()) return;

            int iCountDB = dgvDBPath.Items.Count;
            if (iCountDB == 1) {
                AddToEventLog("Не указаны файлы БД!", true, true);
                return;
            }

            int iCountNS = dgvNameSpace.Items.Count;
            if (iCountNS == 1) {
                AddToEventLog("Не указаны файлы ПИ!", true, true);
                return;
            }

            /*Path to NameSpace directory*/
            String sNSDirPath;

            /*Если загружаем обычные ПИ, то создаём работаем в папке sNSDirPath*/
            /*Если загружаем пакеты, то работаем в папке sNSPath*/
            if (!isPackage & sNSPath == String.Empty)
            {
                sNSDirPath = sCurrentDir + "\\GS_APP";

                if (DirectoryExists((sNSDirPath)))
                {
                    AddToEventLog("Удаление " + sNSDirPath, false, false);

                    if (DeleteDirectory(sNSDirPath))
                    {
                        AddToEventLog("Удаление успешно завершено.", false, false);
                    }
                    else {
                        AddToEventLog("Ошибка при удалении " + sNSDirPath + ". "+ GetLastErrorDescription(), true, false);
                        return;
                    }
                }

                AddToEventLog("Создание " + sNSDirPath, false, false);

                try {
                    Directory.CreateDirectory(sNSDirPath);
                    AddToEventLog("Создание успешно завершено.", false, false);
                }
                catch {
                    AddToEventLog("Ошибка при создании " + sNSDirPath + ". " + GetLastErrorDescription(), true, false);
                }
            }
            else
                sNSDirPath = sNSPath;


            /*Create YAML file****************************************/
            AddToEventLog("Содание пакета ПИ.", false, false);

            String sNSFileName = sNSDirPath + "\\GS.Temp.yml";

            try {
                if (File.Exists(sNSFileName)) File.Delete(sNSFileName);
            }
            catch
            {
                AddToEventLog("Ошибка при удалении " + sNSFileName + ". " + GetLastErrorDescription(), true, false);
            }

            MemoryStream stream = new MemoryStream();
            StreamWriter MS = new StreamWriter(stream, Encoding.UTF8);

            MS.WriteLine("Uses: ");

            String sFileName, sFullFilePath;

            m_NameSpace[] ns = aRowsNameSpace();
            m_DataBase[] db = aRowsDataBase();

            int THREAD_COUNT = iCountDB - 1;

            /*Массив потоков*/
            Thread[] Thread_Arr = new Thread[THREAD_COUNT];
            IsInProcess = true;

            for (int i = 0; i < iCountNS - 1; i++){

                sFileName       = ns[i].NSFileName;
                sFullFilePath   = ns[i].NSPath;

                if (!isPackage & sNSPath == String.Empty)
                {
                    if (!copyFile(sFullFilePath, sNSDirPath + "\\" + sFileName))
                    {
                        AddToEventLog("Ошибка при копировании из " + sNSDirPath + "в " + sNSDirPath + "\\" + sFileName + ". " + GetLastErrorDescription(), true, false);
                        return;
                    }
                }

                MS.WriteLine("  - \"" + GetRUIDNSFile(sFullFilePath) + " " + sFileName + "\"");
            }

            MS.Flush();

            FileStream fs = new FileStream(sNSFileName, FileMode.Append, FileAccess.Write);
            StreamWriter NSFile = new StreamWriter(fs);

            NSFile.WriteLine("%YAML 1.1");
            NSFile.WriteLine("--- ");
            NSFile.WriteLine("StructureVersion: \"1.0\"");
            NSFile.WriteLine("Properties: ");
            NSFile.WriteLine("  RUID: 147666666_666666666");
            NSFile.WriteLine("  Name: \"GS.Temp\"");
            NSFile.WriteLine("  Caption: \"GS.Temp\"");
            NSFile.WriteLine("  Version: \"1.0.0.1\"");
            NSFile.WriteLine("  Optional: False");
            NSFile.WriteLine("  Internal: False");

            /*Идём в начало потока*/
            stream.Position = 0;
            NSFile.WriteLine("  MD5: " + GetMD5((new StreamReader(stream)).ReadToEnd()));
            /*Идём в начало потока*/
            stream.Position = 0;
            NSFile.WriteLine((new StreamReader(stream)).ReadToEnd());

            NSFile.Close();
            /*********************************************************/

            AddToEventLog("Обновление запущено", false, false);

            const String quote = "\"";
            String sDataBase;
            String sCmdLine;

            for (int i = 0; i < THREAD_COUNT; i++){

                sDataBase = db[i].DBPath;

                sCmdLine =
                    quote + sGedeminPath + quote +
                    " /sn " + quote + sDataBase + quote +
                    " /user Administrator" +
                    " /password Administrator" +
                    " /q /SP " +
                    quote + sNSDirPath + quote +
                    " /sfn " +
                    quote + sNSFileName + quote;

                sCmdLine = quote + sCmdLine + quote;

                Thread_Arr[i] = new Thread(() => MyThread(sCmdLine));
                Thread_Arr[i].IsBackground = true;
                Thread_Arr[i].Start();
            }

            /*Поток, который блокирует все действия до завершения всех потоков обновления*/
            Thread MainThread = new Thread(() => MainThreadEvent(Thread_Arr));
            MainThread.IsBackground = true;
            MainThread.Start();
        }

        /************************************************************************/
    }

}
