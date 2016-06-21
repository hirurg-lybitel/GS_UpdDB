#include "frmMain.h"

using namespace GS_DataBaseUpdate;

[STAThreadAttribute]
int main(){
	// Включение визуальных эффектов Windows до создания каких-либо элементов управления
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false);

	// Создание главного окна и его запуск
	Application::Run(gcnew frmMain());
	return 0;
}

TString frmMain::toString(String^ str) {
	return msclr::interop::marshal_as<TString>(str);;
}
TString frmMain::AddStrings(TString &str1, TString &str2) {
	TString tmp;
	tmp.append(str1);
	tmp.append(str2);

	return tmp;
}
TString frmMain::AddStrings(const TCHAR* str1, TString &str2) {
	TString tmp;
	tmp.append(TString(str1));
	tmp.append(str2);

	return tmp;
}
TString frmMain::AddStrings(const TCHAR* str1, const TCHAR* str2) {
	TString tmp;
	tmp.append(TString(str1));
	tmp.append(TString(str2));

	return tmp;
}
TString frmMain::AddStrings(TString &str1, const TCHAR* str2) {
	TString tmp;
	tmp.append(str1);
	tmp.append(TString(str2));

	return tmp;
}
TString frmMain::AddStrings(TString &str1, TString &str2, TString &str3) {
	TString tmp;
	tmp.append(str1);
	tmp.append(str2);
	tmp.append(str3);

	return tmp;
}
TString frmMain::SetDouleSlash(TString &str) {
	TString s = str;
	auto it = find(s.begin(), s.end(), '\\');
	while (it != s.end()) {
		auto it2 = s.insert(it, '\\');
		it = find(it2 + 2, s.end(), '\\');
	}
	return s;
}
System::Void frmMain::AddToEventLog(TString str, BOOL isError, BOOL withMsgBox) {

	if (str == TString()) return void();

	char bufTime[80], bufDate[80];
	time_t t = time(0);
	struct tm now = *localtime(&t);

	strftime(bufTime, sizeof(bufTime), "%X", &now);
	strftime(bufDate, sizeof(bufDate), "%d-%m-%Y", &now);

	dgvEventLog->Rows->Add(
		gcnew String(str.c_str()),
		isError,
		gcnew String(bufTime),
		gcnew String(bufDate));

	dgvEventLog->ClearSelection();

	if (withMsgBox) {
		MessageBox::Show(gcnew String(str.c_str()), "Внимание!", MessageBoxButtons::OK, MessageBoxIcon::Error);
	}

	//delete[] bufTime;
	//delete[] bufDate;
}
System::Void frmMain::btnDBPathAdd_Click(System::Object^  sender, System::EventArgs^  e) {
	ofdDBPath->Title = "Выберите базу данных";
	ofdDBPath->Filter = "Файлы базы данных (*.FDB)|*.FDB|Все файлы (*.*)|*.*";
	ofdDBPath->FileName = "";
	ofdDBPath->InitialDirectory = gcnew String(sLastDBPath.c_str());

	AddToEventLog(L"Выбор базы данных.", false, false);

	if (ofdDBPath->ShowDialog() == System::Windows::Forms::DialogResult::OK) {

		TString tmp = toString(ofdDBPath->FileName);
		string::size_type pos = tmp.find_last_of(L"\\/");
		sLastDBPath = tmp.substr(0, pos);

		DWORD res;
		res = WritePrivateProfileString(
			TEXT("PathSection"),
			searchDBPath.c_str(),
			sLastDBPath.c_str(),
			AddStrings(sCurrentDir, TString(L"\\Settings.ini")).c_str());

		if (res == 0) {
			AddToEventLog(GetLastErrorDescription(), true, false);
		}

		dgvDBPath->Rows->Add(dgvDBPath->Rows->Count, ofdDBPath->FileName);

		AddToEventLog(GetLastErrorDescription(), true, false);
	}
}
System::Void frmMain::btnDBPathDelete_Click(System::Object^  sender, System::EventArgs^  e) {

	int iRecCount = dgvDBPath->Rows->Count;

	if (iRecCount <= 1) return void();

	int iCurRow = dgvDBPath->CurrentCell->RowIndex;

	if ((iCurRow + 1) == iRecCount) return void();

	dgvDBPath->Rows->RemoveAt(dgvDBPath->CurrentCell->RowIndex);

	AddToEventLog(GetLastErrorDescription(), true, false);

	/*Delete all rows*/
	//while (iRecCount > 1) dgvDBPath->Rows->RemoveAt(0);
}
System::Void frmMain::btnNameSpaceAdd_Click(System::Object^  sender, System::EventArgs^  e) {
	ofdNameSpace->Title = "Выберите файлы ПИ";
	ofdNameSpace->Filter = "Файлы YML|*.yml";
	ofdNameSpace->FileName = "";
	ofdNameSpace->Multiselect = true;
	if (sNSPath != TString())
		ofdNameSpace->InitialDirectory = gcnew String(sNSPath.c_str());
	else
		ofdNameSpace->InitialDirectory = gcnew String(sLastNSPath.c_str());

	AddToEventLog(L"Выбор файлов ПИ.", false, false);

	if (ofdNameSpace->ShowDialog() == System::Windows::Forms::DialogResult::OK) {

		TString tmp = toString(ofdNameSpace->FileName);
		string::size_type pos = tmp.find_last_of(L"\\/");
		sLastNSPath = tmp.substr(0, pos);

		DWORD res;
		res = WritePrivateProfileString(
			TEXT("PathSection"),
			searchNSPath.c_str(),
			sLastNSPath.c_str(),
			AddStrings(sCurrentDir, TString(L"\\Settings.ini")).c_str());

		if (res == 0) {
			AddToEventLog(GetLastErrorDescription(), true, false);
		}

		int iFileCount = ofdNameSpace->SafeFileNames->Length;

		for (int i = 0; i < iFileCount; i++) {
			dgvNameSpace->Rows->Add(dgvNameSpace->Rows->Count, ofdNameSpace->SafeFileNames[i], ofdNameSpace->FileNames[i]);

			AddToEventLog(GetLastErrorDescription(), true, false);
		}
	}
}
System::Void frmMain::btnNameSpaceDelete_Click(System::Object^  sender, System::EventArgs^  e) {

	int iRecCount = dgvNameSpace->Rows->Count;

	if (iRecCount <= 1) return void();

	int iCurRow = dgvNameSpace->CurrentCell->RowIndex;

	if ((iCurRow + 1) == iRecCount) return void();

	dgvNameSpace->Rows->RemoveAt(dgvNameSpace->CurrentCell->RowIndex);

	AddToEventLog(GetLastErrorDescription(), true, false);
}
System::Void frmMain::frmMain_Shown(System::Object^  sender, System::EventArgs^  e) {

	TCHAR buffer[MAX_PATH];

	GetModuleFileName(NULL, (LPWSTR)buffer, MAX_PATH);
	string::size_type pos = TString(buffer).find_last_of(L"\\/");
	sCurrentDir = TString(buffer).substr(0, pos);

	DWORD result;

	result = GetPrivateProfileString(
		TEXT("PathSection"),
		searchDBPath.c_str(),
		NULL,
		buffer,
		MAX_PATH,
		AddStrings(sCurrentDir, TString(L"\\Settings.ini")).c_str());
	if (result > 0)
		sLastDBPath = TString(buffer);
	else
		AddToEventLog(AddStrings(L"Ошибка чтения Settings.ini", GetLastErrorDescription()), true, false);

	result = GetPrivateProfileString(
		TEXT("PathSection"),
		searchNSPath.c_str(),
		NULL,
		buffer,
		MAX_PATH,
		AddStrings(sCurrentDir, TString(L"\\Settings.ini")).c_str());
	if (result > 0)
		sLastNSPath = TString(buffer);
	else
		AddToEventLog(AddStrings(L"Ошибка чтения Settings.ini", GetLastErrorDescription()), true, false);

	result = GetPrivateProfileString(
		TEXT("Options"),
		searchGedeminAfterOpen.c_str(),
		NULL,
		buffer,
		MAX_PATH,
		AddStrings(sCurrentDir, TString(L"\\Settings.ini")).c_str());
	if (result > 0)
		SearchGedeminAfterOpen = (wcscmp(buffer, L"1") == 0 ? TRUE : FALSE);
	else
		AddToEventLog(AddStrings(L"Ошибка чтения Settings.ini", GetLastErrorDescription()), true, false);

	if (SearchGedeminAfterOpen) {
		Application::DoEvents();
		SearchOnAllDrives(L"gedemin.exe");
		AddToEventLog(L"", true, false);
	
		if (sGedeminPath != TString()) {
			tbGedeminPath->Text = gcnew String(sGedeminPath.c_str());
			tbGedeminPath->BackColor = Color::PaleGreen;

			AddToEventLog(AddStrings(L"Путь к gedemin.exe: ", sGedeminPath), false, false);
		}
		else {
			tbGedeminPath->Text = "gedemin.exe не найден";
			tbGedeminPath->BackColor = Color::LightCoral;
		}
	}
}
System::Void frmMain::tsbRun_Click(System::Object^  sender, System::EventArgs^  e) {

	int iCountDB = dgvDBPath->Rows->Count;
	if (iCountDB <= 1) {
		AddToEventLog(L"Не указаны файлы БД!", true, true);
		return void();
	}

	int iCountNS = dgvNameSpace->Rows->Count;
	if (iCountNS <= 1) {
		AddToEventLog(L"Не указаны файлы ПИ!", true, true);
		return void();
	}

	int THREAD_COUNT = iCountDB - 1;
	/*создание массива пустых указателей на потоки, которые создадим дальше*/
	HANDLE* Thread_Arr = new HANDLE[THREAD_COUNT];

	/*Path to NameSpace directory*/
	TString sNSDirPath;

	/*Если загружаем обычные ПИ, то создаём работаем в папке sNSDirPath*/
	/*Если загружаем пакеты, то работаем в папке sNSPath*/
	if (!isPackage & sNSPath == TString()) {
		sNSDirPath = AddStrings(sCurrentDir, TString(L"\\GS_APP"));

		if (DirectoryExists((sNSDirPath))) {
			AddToEventLog(AddStrings(L"Удаление ", sNSDirPath), false, false);
			if (DeleteDirectory(sNSDirPath)) {
				AddToEventLog(L"Удаление успешно завершено.", false, false);
			}
			else {
				AddToEventLog(GetLastErrorDescription(), true, false);
				return void();
			}
		}

		AddToEventLog(AddStrings(L"Создание ", sNSDirPath), false, false);
		if (CreateDirectory(sNSDirPath.c_str(), NULL)) {
			AddToEventLog(L"Создание успешно завершено.", false, false);
		}
		else {
			AddToEventLog(GetLastErrorDescription(), true, false);
		};
	}
	else
		sNSDirPath = sNSPath;

	
	/*Create YAML file****************************************/
	AddToEventLog(L"Содание пакета ПИ.", false, false);

	TString sNSFileName = AddStrings(sNSDirPath, TString(L"\\GS.Temp.yml"));

	stringstream MS;

	MS << "Uses: " << endl;

	const char* ptr;
	TString sFileName, sFullFilePath;
	string sTmp;

	for (int i = 0; i < iCountNS - 1; i++) {

		TString sFileName = msclr::interop::marshal_as<TString>((String^)dgvNameSpace->Rows[i]->Cells[1]->Value);
		TString sFullFilePath = msclr::interop::marshal_as<TString>((String^)dgvNameSpace->Rows[i]->Cells[2]->Value);

		if (!isPackage & sNSPath == TString()) {
			if (!copyFile(sFullFilePath.c_str(), AddStrings(AddStrings(sNSDirPath, TString(L"\\")), sFileName).c_str())) {
				AddToEventLog(GetLastErrorDescription(), true, false);
				return void();
			}
		}

		ptr = (const char*)(Marshal::StringToHGlobalAnsi((String^)dgvNameSpace->Rows[i]->Cells[1]->Value)).ToPointer();
		string sTmp = string(ptr);

		MS << "  - \"" + GetRUIDNSFile(sFullFilePath) + " " + sTmp + "\"" << endl;
	}

	ofstream NSFile(sNSFileName.c_str(), ios::out | ios::binary);
	NSFile.imbue(locale(NSFile.getloc(), new codecvt_utf8_utf16<char>));

	NSFile << "%YAML 1.1" << endl;
	NSFile << "--- " << endl;
	NSFile << "StructureVersion: \"1.0\"" << endl;
	NSFile << "Properties: " << endl;
	NSFile << "  RUID: 147666666_666666666" << endl;
	NSFile << "  Name: \"GS.Temp\"" << endl;
	NSFile << "  Caption: \"GS.Temp\"" << endl;
	NSFile << "  Version: \"1.0.0.1\"" << endl;
	NSFile << "  Optional: False" << endl;
	NSFile << "  Internal: False" << endl;
	NSFile << "  MD5: " + GetMD5(MS) << endl;
	NSFile << MS.str();

	NSFile.close();

	AddToEventLog(GetLastErrorDescription(), true, false);
	/*********************************************************/

	ForThread *SendPr;
	TString s;

	for (int i = 0; i < THREAD_COUNT; i++) {
		SendPr = new ForThread;

		s = AddStrings(quote, msclr::interop::marshal_as<TString>((String^)dgvDBPath->Rows[i]->Cells[1]->Value), quote);

		basic_ostringstream<TCHAR> out;

		out << AddStrings(quote, sGedeminPath, quote).c_str()
			<< " /sn " << s.c_str()
			<< " /user Administrator"
			<< " /password Administrator"
			<< " /q /SP "
			<< AddStrings(quote, sNSDirPath, quote).c_str()
			<< " /sfn "
			<< AddStrings(quote, sNSFileName, quote).c_str()
			<< endl;

		SendPr->str = out.str();
		//toString(Convert::ToString(i)
		AddToEventLog(L"Обновление запущено", false, false);

		Thread_Arr[i] = CreateThread(0, 0, MyThread, SendPr, 0, 0);

		if (Thread_Arr[i] == NULL) {
			AddToEventLog(GetLastErrorDescription(), true, false);
		}
	}

	WaitForMultipleObjects(THREAD_COUNT, Thread_Arr, TRUE, INFINITE);

	for (int i = 0; i < iCountDB - 1; i++) {
		CloseHandle(Thread_Arr[i]);
		AddToEventLog(L"Обновление завершено", false, false);
	}
}
System::Void frmMain::btnChooseEXE_Click(System::Object^  sender, System::EventArgs^  e) {

	ofdGedeminPath->Title = "Выберите файл gedemin.exe";
	ofdGedeminPath->Filter = "Файлы EXE|*.exe";
	ofdGedeminPath->FileName = "gedemin.exe";
	ofdNameSpace->RestoreDirectory = true;

	if (ofdNameSpace->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
		sGedeminPath = msclr::interop::marshal_as<TString>((String^)ofdNameSpace->FileName);

		tbGedeminPath->Text = gcnew String(sGedeminPath.c_str());
		tbGedeminPath->BackColor = Color::PaleGreen;

		DWORD res;
		res = WritePrivateProfileString(
			TEXT("PathSection"),
			searchGedeminPath.c_str(),
			sGedeminPath.c_str(),
			AddStrings(sCurrentDir, TString(L"\\Settings.ini")).c_str());

		if (res == 0) {
			AddToEventLog(GetLastErrorDescription(), true, false);
		}
	}
}
System::Void frmMain::tsmiExit_Click(System::Object^  sender, System::EventArgs^  e) {
	System::Windows::Forms::DialogResult Result;
	Result = MessageBox::Show("Вы точно хотите выйти?", "Внимание!", MessageBoxButtons::YesNo, MessageBoxIcon::Question);

	if (Result == System::Windows::Forms::DialogResult::Yes)
		Application::Exit();
}
System::Void frmMain::frmMain_FormClosing(System::Object^  sender, System::Windows::Forms::FormClosingEventArgs^  e) {
	__super::OnClosing(e);
	if (e->CloseReason == System::Windows::Forms::CloseReason::ApplicationExitCall)
		Application::Exit();
	else {
		if (MessageBox::Show("Вы точно хотите выйти?", "Выход", MessageBoxButtons::YesNo,
			MessageBoxIcon::Question) == System::Windows::Forms::DialogResult::No)
			e->Cancel = true;
		else
			Application::Exit();
	}
}
System::Void frmMain::dgvEventLog_RowsAdded(System::Object^  sender, System::Windows::Forms::DataGridViewRowsAddedEventArgs^  e) {

	dgvEventLog->Rows[e->RowIndex]->DefaultCellStyle->ForeColor = Color::Blue;

	if (Convert::ToInt16(dgvEventLog->Rows[e->RowIndex]->Cells[1]->Value->ToString()) == 1)
		dgvEventLog->Rows[e->RowIndex]->DefaultCellStyle->ForeColor = Color::Red;

}
System::Void frmMain::tsmiOptions_Click(System::Object^  sender, System::EventArgs^  e) {

	frmOptions^ frmOpt = gcnew frmOptions();

	if (frmOpt->ShowDialog() == System::Windows::Forms::DialogResult::OK) {

		SearchGedeminAfterOpen = frmOpt->cbSearchGedemin->Checked;

		DWORD res;
		res = WritePrivateProfileString(
			TEXT("Options"),
			searchGedeminAfterOpen.c_str(),
			((SearchGedeminAfterOpen ? TString(L"1") : TString(L"0"))).c_str(),
			AddStrings(sCurrentDir, TString(L"\\Settings.ini")).c_str());

		if (res == 0) {
			AddToEventLog(GetLastErrorDescription(), true, false);
		}

	}
}
System::Void frmMain::tsmiAbout_Click(System::Object^  sender, System::EventArgs^  e) {
	frmAboutProgram^ frmAbout = gcnew frmAboutProgram();
	frmAbout->ShowDialog();
}
System::Void frmMain::tsmiClear_Click(System::Object^  sender, System::EventArgs^  e) {
	dgvEventLog->Rows->Clear();
}
System::Void frmMain::cbIsPackage_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
	isPackage = cbIsPackage->Checked;
	pnlNS->Visible = isPackage;

	if (!isPackage) {
		sNSPath = TString();
		tbNSPath->Text = "";
	}
}
System::Void frmMain::btnChooseNSPath_Click(System::Object^  sender, System::EventArgs^  e) {

	AddToEventLog(L"Выбор папки ПИ.", false, false);
	if (fbdNameSpaceFolder->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
		tbNSPath->Text = fbdNameSpaceFolder->SelectedPath;

		sNSPath = toString(fbdNameSpaceFolder->SelectedPath);
	}
		
}