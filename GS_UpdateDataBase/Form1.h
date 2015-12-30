#pragma once
#include <windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <tchar.h>
#include <vcclr.h>
#include <codecvt>

#include <msclr\marshal.h>
#include <msclr\marshal_cppstd.h>

#include <ObjBase.h>
#pragma comment(lib,"ole32.lib")

#include <Psapi.h>
#pragma comment(lib,"psapi.lib")

#include "MyConvecter.h"

using namespace std;

#ifndef UNICODE  
typedef string TString; 
#else
typedef wstring TString; 
#endif

#define quote TString(L"\"")

HWND g_HWND = NULL;

TString sGedeminPath = TString();
TString sCurrentDir;
TString sLastDBPath = TString();
TString sLastNSPath = TString();

struct ForThread{
	TString str;
	const TCHAR* NSFileName;
	const TCHAR* NSPath;
	const TCHAR* DBPath;
	const TCHAR* GedeminPath;
};

BOOL CALLBACK EnumWindowsProcMy(HWND hwnd,LPARAM lParam){	
	DWORD lpdwProcessId;
	GetWindowThreadProcessId(hwnd,&lpdwProcessId);
	if(lpdwProcessId==lParam)
	{
		g_HWND=hwnd;
		return FALSE;
	}
	return TRUE;
}

BOOL DirectoryExists(const TString& sNSDirPath){
	DWORD fa = GetFileAttributes(sNSDirPath.c_str());

	if (fa == INVALID_FILE_ATTRIBUTES){
		return false;
	}

	if (fa & FILE_ATTRIBUTE_DIRECTORY){
		return true;
	} 

	return false;
}

TString GetLastErrorDescription(DWORD errCode = 0){

	DWORD errorMessageID;
	if (errCode == 0){
		errorMessageID = GetLastError();
		if(errorMessageID == 0)
			return TString(); 
	}
	else
		errorMessageID = errCode;

	LPWSTR messageBuffer = NULL;
	size_t size = FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, 
		errorMessageID, 
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
		(LPWSTR)&messageBuffer, 
		0,
		NULL
		);

	TString message(messageBuffer, size);

	LocalFree(messageBuffer);

	return message;
}

BOOL DeleteDirectory(const TString& sDir, bool bDeleteSubDir = true){

	bool bWasSubDir = false;   
	HANDLE hDir;                       
	TString sFilePath;                 
	TString sPattern;                  
	WIN32_FIND_DATA FileInformation;             

	sPattern.append(sDir);
	sPattern.append(L"\\*.*");

	hDir = FindFirstFile(sPattern.c_str(), &FileInformation);
	if(hDir != INVALID_HANDLE_VALUE){
		do{
			if(FileInformation.cFileName[0] != '.'){
				sFilePath.erase();

				sFilePath.clear();
				sFilePath.append(sDir);
				sFilePath.append(L"\\");
				sFilePath.append(FileInformation.cFileName);

				if(FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
					if(bDeleteSubDir){
						// Delete subdirectory
						if(!DeleteDirectory(sFilePath, bDeleteSubDir))
							return false;
					}
					else
						bWasSubDir = true;
				}
				else{
					if(SetFileAttributes(sFilePath.c_str(), FILE_ATTRIBUTE_NORMAL) == FALSE)
						return false;

					if(DeleteFile(sFilePath.c_str()) == FALSE) return false;
				}
			}
		} while(FindNextFile(hDir, &FileInformation) == TRUE);

		//ERROR_NO_MORE_FILES
		if (!FindClose(hDir)){
			return false;
		}
		else{
			if(!bWasSubDir){
				if(SetFileAttributes(sDir.c_str(), FILE_ATTRIBUTE_NORMAL) == FALSE)
					return false;

				if(RemoveDirectory(sDir.c_str()) == FALSE)
					return false;
			}
		}
	}

	return true;
}

string IntToHex(int num)
{
	ostringstream stream;
	stream << hex << num;
	return stream.str();
}

string GetMD5(stringstream& MS){
	HCRYPTPROV hProv;
	HCRYPTHASH Hash;
	BYTE HashValue[16];
	DWORD HashValueSize;
	string HashString;

	if (!CryptAcquireContext(&hProv, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) return string();
	if (!CryptCreateHash(hProv, CALG_MD5, 0, 0, &Hash)) return string();

	HashValueSize = sizeof(HashValue);

	MS.seekg(0, ios::end);
	if (CryptHashData(Hash, (BYTE*)MS.str().c_str(),  MS.tellg(), 0) && CryptGetHashParam(Hash, HP_HASHVAL, HashValue, &HashValueSize, 0)){
		for (int i = 0; i < HashValueSize - 1; i++){
			HashString += IntToHex(HashValue[i]);
		}
	}
	else return string();
	if (!CryptDestroyHash(Hash)) return string();
	if (!CryptReleaseContext(hProv, 0)) return string();

	transform(HashString.begin(), HashString.end(), HashString.begin(), toupper);
	return HashString;
}

string GetRUIDNSFile(TString& FileName){
	string search = "RUID"; 
	string result = string(); 
	string line;

	ifstream fileInput;
	fileInput.open(FileName.c_str());
	string::size_type pos;

	while(getline(fileInput, line)){ 
		pos = line.find(search, 0);
		if (pos != string::npos) {			
			result = line.substr(pos+6, line.size());
			break;
		}
	}
	return result;
}

BOOL copyFile(const TCHAR* cFrom, const TCHAR* cTo){
	ifstream src(cFrom, ios::binary);
	ofstream dest(cTo, ios::binary);
	dest << src.rdbuf();
	return src && dest;
}

VOID ProcessFoundFile(LPWSTR CONST szPath, WIN32_FIND_DATA CONST * CONST fdFindData, LPWSTR CONST lpSearch){
	//TCHAR szEnd[] = L"\r\n";
	//DWORD dwTemp;
	if (wcscmp(fdFindData->cFileName, lpSearch) == 0){ 
		//WriteConsole(hStdOut, szPath, lstrlen(szPath), &dwTemp, NULL);
		//WriteConsole(hStdOut, szEnd, STRLEN(szEnd), &dwTemp, NULL);
		sGedeminPath = TString(szPath);
		//return TString(szPath);
	}
	//return TString();
}
/*
VOID FindFirstFileFailed(LPWSTR CONST szPath){
	TCHAR CONST szMsgTmpl[] = L"FindFirstFile() failed, " L"GetLastError() = %d, szPath = %s\r\n";
	TCHAR szMsg[MAX_PATH*2];
	DWORD dwTemp;
	//wsprintf(szMsg, szMsgTmpl, GetLastError(), szPath);
	//WriteConsole(hStdOut, szMsg, lstrlen(szMsg), &dwTemp, NULL);
}
*/
VOID RecursiveSearch(LPWSTR szPath, LPWSTR CONST lpSearch, CONST BOOL OnlyFirst){
	WIN32_FIND_DATA fdFindData;
	HANDLE hFind;
	TCHAR * CONST lpLastChar = szPath + lstrlen(szPath);

	lstrcat(szPath, L"*");
	hFind = FindFirstFile(szPath, &fdFindData);
	*lpLastChar = '\0';

	if(INVALID_HANDLE_VALUE == hFind) {
		//FindFirstFileFailed(szPath);
		return;
	}

	do{
		if((0 == lstrcmp(fdFindData.cFileName, L".")) ||
			(0 == lstrcmp(fdFindData.cFileName, L".."))){
				continue;
		}
		lstrcat(szPath, fdFindData.cFileName);
		if(fdFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
			lstrcat(szPath, L"\\");
			RecursiveSearch(szPath, lpSearch, OnlyFirst);
		} else {
			ProcessFoundFile(szPath, &fdFindData, lpSearch);
		}
		*lpLastChar = '\0';

		if (OnlyFirst && sGedeminPath != TString()) return void();

	} while(FindNextFile(hFind, &fdFindData));

	FindClose(hFind);
}

VOID SearchOnAllDrives(LPWSTR CONST lpSearch, CONST BOOL OnlyFirst = true){
	TCHAR szCurrDrive[] = L"A:\\";
	TCHAR szPath[MAX_PATH*2+1];
	DWORD i, dwDisksMask = GetLogicalDrives();

	int N=0;
	for(i = 0; i < 26; i++){
		if(dwDisksMask & 1){
			lstrcpy(szPath, szCurrDrive);
			RecursiveSearch(szPath, lpSearch, OnlyFirst);
			if (OnlyFirst && sGedeminPath != TString()) return void();
		}
		dwDisksMask >>= 1; 
		szCurrDrive[0]++;
	}
}

string ConvertToString(TString& wStr){
	int max_len = wStr.size();
	string str; 
	str.resize(max_len);
	WideCharToMultiByte(CP_ACP, 0, wStr.c_str(), -1, (LPSTR)str.c_str(), max_len, NULL, NULL);
	return str;
}

DWORD WINAPI MyThread(LPVOID p){

	ForThread* in = reinterpret_cast<ForThread*>(p);
	TString str = in->str;

	TCHAR tempCmdLine[MAX_PATH * 2];
	_tcscpy_s(tempCmdLine, MAX_PATH *2, str.c_str());

	STARTUPINFO si = {sizeof(si)};
	PROCESS_INFORMATION pi;			
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;

	if(CreateProcess(NULL, tempCmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)){
		WaitForSingleObject(pi.hProcess, INFINITE);
		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );
	}
	else{
		TString mes = GetLastErrorDescription();
	}

	delete in; //освобождаю память под структуру
	return 0;
}



namespace GS_UpdateDataBase {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Text;
	using namespace System::Diagnostics;
	using namespace System::Threading;
	using namespace System::Runtime::InteropServices;

	public ref class Form1 : public System::Windows::Forms::Form{
	public:
		Form1(void){
			InitializeComponent();
		}

	protected:
		~Form1(){
			if (components){
				delete components;
			}
		}


	private: System::Windows::Forms::Button^  button1;
	private: System::Windows::Forms::DataGridView^  dgvDBPath;
	private: System::Windows::Forms::DataGridView^  dgvNameSpace;
	private: System::Windows::Forms::Button^  btnDBPathAdd;
	private: System::Windows::Forms::Button^  btnDBPathDelete;
	private: System::Windows::Forms::OpenFileDialog^  ofdDBPath;

	private: System::Windows::Forms::OpenFileDialog^  ofdNameSpace;
	private: System::Windows::Forms::Button^  btnNameSpaceDelete;
	private: System::Windows::Forms::Button^  btnNameSpaceAdd;


	private: System::Windows::Forms::Button^  btnRun;
	private: System::Windows::Forms::TextBox^  tbGedeminPath;

	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column2;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  DBPath;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column3;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column1;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  NameSpacePath;
	private: System::Windows::Forms::Button^  btnSearch;



	protected: 

	private:
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		void InitializeComponent(void){
			System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(Form1::typeid));
			this->button1 = (gcnew System::Windows::Forms::Button());
			this->dgvDBPath = (gcnew System::Windows::Forms::DataGridView());
			this->Column2 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->DBPath = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->dgvNameSpace = (gcnew System::Windows::Forms::DataGridView());
			this->Column3 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column1 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->NameSpacePath = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->btnDBPathAdd = (gcnew System::Windows::Forms::Button());
			this->btnDBPathDelete = (gcnew System::Windows::Forms::Button());
			this->ofdDBPath = (gcnew System::Windows::Forms::OpenFileDialog());
			this->ofdNameSpace = (gcnew System::Windows::Forms::OpenFileDialog());
			this->btnNameSpaceDelete = (gcnew System::Windows::Forms::Button());
			this->btnNameSpaceAdd = (gcnew System::Windows::Forms::Button());
			this->btnRun = (gcnew System::Windows::Forms::Button());
			this->tbGedeminPath = (gcnew System::Windows::Forms::TextBox());
			this->btnSearch = (gcnew System::Windows::Forms::Button());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->dgvDBPath))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->dgvNameSpace))->BeginInit();
			this->SuspendLayout();
			// 
			// button1
			// 
			this->button1->Location = System::Drawing::Point(650, 317);
			this->button1->Name = L"button1";
			this->button1->Size = System::Drawing::Size(75, 23);
			this->button1->TabIndex = 0;
			this->button1->Text = L"button1";
			this->button1->UseVisualStyleBackColor = true;
			this->button1->Click += gcnew System::EventHandler(this, &Form1::button1_Click);
			// 
			// dgvDBPath
			// 
			this->dgvDBPath->CellBorderStyle = System::Windows::Forms::DataGridViewCellBorderStyle::SingleVertical;
			this->dgvDBPath->ColumnHeadersHeightSizeMode = System::Windows::Forms::DataGridViewColumnHeadersHeightSizeMode::AutoSize;
			this->dgvDBPath->Columns->AddRange(gcnew cli::array< System::Windows::Forms::DataGridViewColumn^  >(2) {this->Column2, this->DBPath});
			this->dgvDBPath->Location = System::Drawing::Point(12, 56);
			this->dgvDBPath->MultiSelect = false;
			this->dgvDBPath->Name = L"dgvDBPath";
			this->dgvDBPath->ScrollBars = System::Windows::Forms::ScrollBars::Vertical;
			this->dgvDBPath->Size = System::Drawing::Size(428, 150);
			this->dgvDBPath->TabIndex = 1;
			// 
			// Column2
			// 
			this->Column2->HeaderText = L"№ п/п";
			this->Column2->Name = L"Column2";
			this->Column2->Width = 30;
			// 
			// DBPath
			// 
			this->DBPath->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::Fill;
			this->DBPath->HeaderText = L"Путь к базе данных";
			this->DBPath->Name = L"DBPath";
			// 
			// dgvNameSpace
			// 
			this->dgvNameSpace->CellBorderStyle = System::Windows::Forms::DataGridViewCellBorderStyle::SingleVertical;
			this->dgvNameSpace->ColumnHeadersHeightSizeMode = System::Windows::Forms::DataGridViewColumnHeadersHeightSizeMode::AutoSize;
			this->dgvNameSpace->Columns->AddRange(gcnew cli::array< System::Windows::Forms::DataGridViewColumn^  >(3) {this->Column3, 
				this->Column1, this->NameSpacePath});
			this->dgvNameSpace->Location = System::Drawing::Point(12, 264);
			this->dgvNameSpace->MultiSelect = false;
			this->dgvNameSpace->Name = L"dgvNameSpace";
			this->dgvNameSpace->Size = System::Drawing::Size(632, 150);
			this->dgvNameSpace->TabIndex = 2;
			// 
			// Column3
			// 
			this->Column3->Frozen = true;
			this->Column3->HeaderText = L"№ п/п";
			this->Column3->Name = L"Column3";
			this->Column3->ReadOnly = true;
			this->Column3->Width = 30;
			// 
			// Column1
			// 
			this->Column1->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
			this->Column1->Frozen = true;
			this->Column1->HeaderText = L"Имя файла ПИ";
			this->Column1->Name = L"Column1";
			this->Column1->Width = 589;
			// 
			// NameSpacePath
			// 
			this->NameSpacePath->HeaderText = L"NameSpacePath";
			this->NameSpacePath->Name = L"NameSpacePath";
			this->NameSpacePath->Visible = false;
			// 
			// btnDBPathAdd
			// 
			this->btnDBPathAdd->BackColor = System::Drawing::Color::Transparent;
			this->btnDBPathAdd->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnDBPathAdd.Image")));
			this->btnDBPathAdd->Location = System::Drawing::Point(12, 10);
			this->btnDBPathAdd->Name = L"btnDBPathAdd";
			this->btnDBPathAdd->Size = System::Drawing::Size(44, 40);
			this->btnDBPathAdd->TabIndex = 3;
			this->btnDBPathAdd->UseVisualStyleBackColor = false;
			this->btnDBPathAdd->Click += gcnew System::EventHandler(this, &Form1::btnDBPathAdd_Click);
			this->btnDBPathAdd->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &Form1::button2_Paint);
			// 
			// btnDBPathDelete
			// 
			this->btnDBPathDelete->BackColor = System::Drawing::Color::Transparent;
			this->btnDBPathDelete->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnDBPathDelete.Image")));
			this->btnDBPathDelete->Location = System::Drawing::Point(62, 10);
			this->btnDBPathDelete->Name = L"btnDBPathDelete";
			this->btnDBPathDelete->Size = System::Drawing::Size(44, 40);
			this->btnDBPathDelete->TabIndex = 4;
			this->btnDBPathDelete->UseVisualStyleBackColor = false;
			this->btnDBPathDelete->Click += gcnew System::EventHandler(this, &Form1::btnDBPathDelete_Click);
			// 
			// btnNameSpaceDelete
			// 
			this->btnNameSpaceDelete->BackColor = System::Drawing::Color::Transparent;
			this->btnNameSpaceDelete->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnNameSpaceDelete.Image")));
			this->btnNameSpaceDelete->Location = System::Drawing::Point(62, 218);
			this->btnNameSpaceDelete->Name = L"btnNameSpaceDelete";
			this->btnNameSpaceDelete->Size = System::Drawing::Size(44, 40);
			this->btnNameSpaceDelete->TabIndex = 6;
			this->btnNameSpaceDelete->UseVisualStyleBackColor = false;
			this->btnNameSpaceDelete->Click += gcnew System::EventHandler(this, &Form1::btnNameSpaceDelete_Click);
			// 
			// btnNameSpaceAdd
			// 
			this->btnNameSpaceAdd->BackColor = System::Drawing::Color::Transparent;
			this->btnNameSpaceAdd->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnNameSpaceAdd.Image")));
			this->btnNameSpaceAdd->Location = System::Drawing::Point(12, 218);
			this->btnNameSpaceAdd->Name = L"btnNameSpaceAdd";
			this->btnNameSpaceAdd->Size = System::Drawing::Size(44, 40);
			this->btnNameSpaceAdd->TabIndex = 5;
			this->btnNameSpaceAdd->UseVisualStyleBackColor = false;
			this->btnNameSpaceAdd->Click += gcnew System::EventHandler(this, &Form1::btnNameSpaceAdd_Click);
			// 
			// btnRun
			// 
			this->btnRun->Location = System::Drawing::Point(632, 80);
			this->btnRun->Name = L"btnRun";
			this->btnRun->Size = System::Drawing::Size(75, 23);
			this->btnRun->TabIndex = 7;
			this->btnRun->Text = L"button2";
			this->btnRun->UseVisualStyleBackColor = true;
			this->btnRun->Click += gcnew System::EventHandler(this, &Form1::btnRun_Click);
			// 
			// tbGedeminPath
			// 
			this->tbGedeminPath->BackColor = System::Drawing::SystemColors::Window;
			this->tbGedeminPath->Cursor = System::Windows::Forms::Cursors::IBeam;
			this->tbGedeminPath->Enabled = false;
			this->tbGedeminPath->Font = (gcnew System::Drawing::Font(L"Times New Roman", 15, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Bold | System::Drawing::FontStyle::Italic)), 
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(204)));
			this->tbGedeminPath->Location = System::Drawing::Point(263, 10);
			this->tbGedeminPath->Name = L"tbGedeminPath";
			this->tbGedeminPath->Size = System::Drawing::Size(296, 30);
			this->tbGedeminPath->TabIndex = 8;
			this->tbGedeminPath->Text = L"Поиск gedemin.exe...";
			// 
			// btnSearch
			// 
			this->btnSearch->BackColor = System::Drawing::Color::Transparent;
			this->btnSearch->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"btnSearch.Image")));
			this->btnSearch->Location = System::Drawing::Point(565, 10);
			this->btnSearch->Name = L"btnSearch";
			this->btnSearch->Size = System::Drawing::Size(30, 30);
			this->btnSearch->TabIndex = 9;
			this->btnSearch->UseVisualStyleBackColor = false;
			this->btnSearch->Click += gcnew System::EventHandler(this, &Form1::btnSearch_Click);
			// 
			// Form1
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(874, 510);
			this->Controls->Add(this->btnSearch);
			this->Controls->Add(this->tbGedeminPath);
			this->Controls->Add(this->btnRun);
			this->Controls->Add(this->btnNameSpaceDelete);
			this->Controls->Add(this->btnNameSpaceAdd);
			this->Controls->Add(this->btnDBPathDelete);
			this->Controls->Add(this->btnDBPathAdd);
			this->Controls->Add(this->dgvNameSpace);
			this->Controls->Add(this->dgvDBPath);
			this->Controls->Add(this->button1);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
			this->Icon = (cli::safe_cast<System::Drawing::Icon^  >(resources->GetObject(L"$this.Icon")));
			this->Name = L"Form1";
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
			this->Text = L"Обновление БД";
			this->Shown += gcnew System::EventHandler(this, &Form1::Form1_Shown);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->dgvDBPath))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->dgvNameSpace))->EndInit();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	private: System::Void button1_Click(System::Object^  sender, System::EventArgs^  e) {

				 CoInitializeEx(0, COINIT_MULTITHREADED); 
				 CLSID clsid; 
				 HRESULT hr = CLSIDFromProgID(L"Gedemin.gsGedeminApplication", &clsid);

				 if(FAILED(hr)) { 
					 MessageBox::Show("Неудача CLSIDFromProgID!", "Внимание!", MessageBoxButtons::OK, MessageBoxIcon::Error);
					 return void();
				 } 

				 IDispatch *pWApp;
				 hr = CoCreateInstance(clsid, NULL, CLSCTX_ALL,IID_IUnknown, (void **)&pWApp);
				 if(FAILED(hr)) { 
					 MessageBox::Show("Неудача CoCreateInstance!", "Внимание!", MessageBoxButtons::OK, MessageBoxIcon::Error);
					 return void(); 
				 }
				 if (SUCCEEDED(hr)){
					 MessageBox::Show("Успешно CoCreateInstance!", "Внимание!", MessageBoxButtons::OK, MessageBoxIcon::Information);


				 }



				 return void();
				 /*Создаёт перечисленные ниже процессы и выводит количество используемой и 
				 выделенной памяти(по аналогии с информаицей выводимой в диспетчере задач)*/
				 //panelPipe->Visible = false;
				 TCHAR pr1[] = L"Notepad";
				 TCHAR pr2[] = L"Calc";
				 TCHAR pr3[] = L"MsPaint";
				 TCHAR pr4[] = L"Fxscover";
				 TCHAR pr5[] = L"Charmap";
				 TCHAR pr6[] = L"Xpsrchvw";

				 TCHAR ged[] = L"D:\\Gedemin\\gedemin.exe /sn D:\\Gedemin\\DataBase\\ETALON.FDB /user Administrator /password Administrator";
				 // TCHAR ged[] = L"Notepad";
				 //char g[] = "D:\Gedemin\gedemin.exe /sn D:\Работа\Базы\Test\ETALON.FDB /user Administrator /password Administrator";
				 // OemToCharA(g, g);

				 // ged = g;

				 const TCHAR* process[] = {pr1, pr2, pr3, pr4, pr5, pr6};		//создаём массив процессов

				 STARTUPINFO si = {sizeof(si)};
				 PROCESS_INFORMATION pi;												
				 System::String^ pName;

				 si.dwFlags = STARTF_USESHOWWINDOW;
				 si.wShowWindow = SW_HIDE;

				 MEMORYSTATUS MemStat;
				 GlobalMemoryStatus(&MemStat);									//MSDN - получение информации об ОЗУ

				 // this->textBox->Text = "Общая память ПК: " + DigitMask(MemStat.dwTotalPhys/1024) + " кБ\r\n";
				 // this->textBox->Text = this->textBox->Text + "==============================================\r\n\r\n";

				 int i = 0;
				 // for (int i = 0; i < CountProc; i++){
				 /*Создаем процесс, имя передаем через командную строку
				 Если создание прошло удачно, то выводим дескриптор и идентификатор*/
				 if(CreateProcess(NULL, (LPWSTR)ged, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)){		//MSDN - создание процесса

					 pName = gcnew String(ged);		//Преобразование название процесса к типу String^ для вывода в textBox 
					 // this->textBox->Text = this->textBox->Text + "Процесс " + pName + " успешно создан!\r\n\r\n";
					 // this->textBox->Text = this->textBox->Text + "Дескриптор процесса: " + ToSysStrFun(pi.hProcess) + "\r\n";
					 // this->textBox->Text = this->textBox->Text + "Идентификатор процесса: " + ToSysStrFun(pi.dwProcessId) + "\r\n\r\n";


					 HANDLE H;		
					 H = OpenProcess(PROCESS_QUERY_INFORMATION |PROCESS_VM_READ |PROCESS_ALL_ACCESS, FALSE, pi.dwProcessId);		//MSDN - возвращает указатель на процесс
					 PROCESS_MEMORY_COUNTERS pmc;

					 pmc.cb = sizeof(pmc);		

					 Sleep(2000);
					 //					 HWND wnd;
					 EnumWindows(EnumWindowsProcMy,pi.dwProcessId);
					 //GetWindowThreadProcessId(wnd, &pi.dwProcessId);

					 if (g_HWND){	
						 char * txt = "1223";							

						 ShowWindow(g_HWND, SW_HIDE);
						 HWND h = FindWindowEx(g_HWND, NULL, TEXT( "TfrmGedeminMain" ), NULL );	
						 //Send text
						 /*HWND h = FindWindowEx(g_HWND, NULL, TEXT( "Edit" ), NULL );							
						 SendMessage( h, WM_SETTEXT, ( WPARAM )TRUE, ( LPARAM )ged );*/

						 // Create a generic keyboard event structure
						 INPUT ip;
						 ip.type = INPUT_KEYBOARD;
						 ip.ki.wScan = 0;
						 ip.ki.time = 0;
						 ip.ki.dwExtraInfo = 0;

						 //Alt - VK_MENU
						 //Shift - VK_SHIFT
						 // Press the "Ctrl" key
						 ip.ki.wVk = VK_CONTROL;							
						 ip.ki.dwFlags = 0; // 0 for key press
						 SendInput(1, &ip, sizeof(INPUT));

						 ip.ki.wVk = VK_SHIFT;							
						 ip.ki.dwFlags = 0; // 0 for key press
						 SendInput(1, &ip, sizeof(INPUT));

						 ip.ki.wVk = VK_MENU;							
						 ip.ki.dwFlags = 0; // 0 for key press
						 SendInput(1, &ip, sizeof(INPUT));

						 // Press the "V" key
						 ip.ki.wVk = 'G';
						 ip.ki.dwFlags = 0; // 0 for key press
						 SendInput(1, &ip, sizeof(INPUT));

						 // Release the "V" key
						 ip.ki.wVk = 'G';
						 ip.ki.dwFlags = KEYEVENTF_KEYUP;
						 SendInput(1, &ip, sizeof(INPUT));

						 ip.ki.wVk = VK_MENU;							
						 ip.ki.dwFlags = KEYEVENTF_KEYUP; // 0 for key press
						 SendInput(1, &ip, sizeof(INPUT));

						 ip.ki.wVk = VK_SHIFT;							
						 ip.ki.dwFlags = KEYEVENTF_KEYUP; // 0 for key press
						 SendInput(1, &ip, sizeof(INPUT));

						 // Release the "Ctrl" key
						 ip.ki.wVk = VK_CONTROL;
						 ip.ki.dwFlags = KEYEVENTF_KEYUP;
						 SendInput(1, &ip, sizeof(INPUT));

						 /*DWORD dwPID = 0;
						 DWORD hThread = GetWindowThreadProcessId(g_HWND, &dwPID);  
						 if (dwPID == pi.dwProcessId && hThread!= NULL ) {
						 PostThreadMessage( hThread, WM_KEYDOWN,'A',1);
						 }*/

						 //PostMessage(g_HWND, WM_KEYDOWN,'A',1); //send
						 //PostMessage(g_HWND, 444, 48 + 23, 0); 
						 /*if (ShowWindow(g_HWND, SW_HIDE)){
						 LRESULT res = SendMessage(g_HWND, 0, 0, 0);
						 MessageBox::Show("Успех!", "Внимание!", MessageBoxButtons::YesNo, MessageBoxIcon::Question);								

						 }
						 else{
						 MessageBox::Show("Неудача!", "Внимание!", MessageBoxButtons::YesNo, MessageBoxIcon::Question);
						 }*/
					 }

					 if (GetProcessMemoryInfo(H, &pmc,sizeof(pmc))){		//MSDN - получение информации о работе процесса с памятью ОЗУ

						 // this->textBox->Text = this->textBox->Text + "Использованная память: " + DigitMask(pmc.WorkingSetSize/1024) + " кБ\r\n";
						 //this->textBox->Text = this->textBox->Text + "PeakWorkingSetSize: " + PMC.PeakWorkingSetSize/1024 + " кБ\r\n";
						 // this->textBox->Text = this->textBox->Text + "Выделенная память: " + DigitMask(pmc.PagefileUsage/1024) + " кБ\r\n";				
					 }
					 else{
						 //MessageBox::Show("Херня!", "Внимание!", MessageBoxButtons::OK, MessageBoxIcon::Error);
					 }
					 CloseHandle( H );		

					 Sleep(500);
					 /* if(PostThreadMessage(pi.dwThreadId, WM_QUIT, 0, 0)){	// MSDN - посылаем указанному потоку комманду закрытия
					 WaitForSingleObject(pi.hProcess, INFINITE);			// ждем завершения процесса
					 CloseHandle(pi.hProcess);							// закрывам дескрипторы 
					 CloseHandle(pi.hThread);							// закрываем поток

					 //	 if(TerminateProcess(pi.hProcess, 0)){//убиваем процесс
					 this->textBox->Text = this->textBox->Text + "Процесс " + pName + " успешно завершён!\r\n";
					 }
					 else{
					 DWORD dwError;
					 if (GetLastError() != ERROR_ACCESS_DENIED){
					 dwError = GetLastError();
					 MessageBox::Show("Процесс не был завершён!", "Внимание!", MessageBoxButtons::OK, MessageBoxIcon::Error);
					 }*/
					 // }

					 // this->textBox->Text = this->textBox->Text + "==============================================\r\n\r\n";
				 }
				 // }

			 }
	private: System::Void button2_Paint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e) {

				 e->Graphics->DrawRectangle(gcnew Pen(this->BackColor, 1), this->ClientRectangle);
			 }
	private: System::Void btnDBPathAdd_Click(System::Object^  sender, System::EventArgs^  e) {

				 ofdDBPath->Title = "Выберите базу данных";
				 ofdDBPath->Filter = "Файлы базы данных (*.FDB, *.GDB)|*.FDB|Все файлы (*.*)|*.*" ;				 			
				 ofdDBPath->FileName = "";
				 ofdDBPath->RestoreDirectory = true;
			     ofdDBPath->InitialDirectory = gcnew String(sLastDBPath.c_str());
				 
				 //TString m = GetLastErrorDescription();

				 if (ofdDBPath->ShowDialog()==System::Windows::Forms::DialogResult::OK){		
			
					 /*TString tmp = msclr::interop::marshal_as<TString>((String^)ofdDBPath->FileName);
					 string::size_type pos = tmp.find_last_of(L"\\/");
					 TString sLastDBPath = tmp.substr( 0, pos);

					 TString line;
					 TString searchDBPath = L"LastDBPath="; 
					 wfstream SettingFile;

					 null_wcodecvt wcodec(1);
					 std::locale wloc(std::locale::classic(), &wcodec);
					 SettingFile.imbue(wloc);

					 SettingFile.open(AddStrings(sCurrentDir, TString(L"\\Settings.ini")), ios::in|ios::out|ios::binary);

					 
					 if(!SettingFile){
						 //File is not open
						 TString m = GetLastErrorDescription();
					 }

					 int m = searchDBPath.size();
					 while(getline(SettingFile, line)){ 
						 pos = line.find(searchDBPath, 0);
						 if (pos != string::npos) {			
							 //SettingFile.seekg(24); //pos + m
							 //SettingFile << AddStrings(AddStrings(TString(L"\""), sLastDBPath), TString(L"\""));
							 SettingFile.write(sLastDBPath.c_str(), sLastDBPath.size());
							 break;
						 }
					 }
					 SettingFile.close();*/

					 dgvDBPath->Rows->Add(dgvDBPath->Rows->Count, ofdDBPath->FileName);
				 }
				 else
					 return void();
			 }
	private: System::Void btnDBPathDelete_Click(System::Object^  sender, System::EventArgs^  e) {

				 int iRecCount = dgvDBPath->Rows->Count;

				 if (iRecCount <= 1) return void();

				 int iCurRow = dgvDBPath->CurrentCell->RowIndex;

				 if ((iCurRow+1) == iRecCount) return void();

				 dgvDBPath->Rows->RemoveAt(dgvDBPath->CurrentCell->RowIndex);

				 /*Delete all rows*/
				 //while (iRecCount > 1) dgvDBPath->Rows->RemoveAt(0);
			 }
	private: System::Void btnNameSpaceAdd_Click(System::Object^  sender, System::EventArgs^  e) {
				 ofdNameSpace->Title = "Выберите файлы ПИ";
				 ofdNameSpace->Filter = "Файлы YML|*.yml";	
				 ofdNameSpace->FileName = "";
				 ofdNameSpace->Multiselect = true;
				 ofdNameSpace->InitialDirectory = gcnew String(sLastNSPath.c_str());
				 //ofdNameSpace->RestoreDirectory = true;

				 if (ofdNameSpace->ShowDialog()==System::Windows::Forms::DialogResult::OK){

					 TString tmp = msclr::interop::marshal_as<TString>((String^)ofdNameSpace->FileName);
					 string::size_type pos = tmp.find_last_of(L"\\/");
					 TString sLastNSPath = tmp.substr( 0, pos);

					 int iFileCount = ofdNameSpace->SafeFileNames->Length;

					 for (int i = 0; i < iFileCount; i++){
						 dgvNameSpace->Rows->Add(dgvNameSpace->Rows->Count, ofdNameSpace->SafeFileNames[i], ofdNameSpace->FileNames[i]);
						 //dgvNameSpace->Rows->Add(ofdNameSpace->FileNames[i]);
					 }					
				 }
				 else
					 return void();
			 }
	private: TString AddStrings(TString &str1, TString &str2){
				 TString tmp;
				 tmp.append(str1);
				 tmp.append(str2);

				 return tmp;
			 }
	private: TString AddStrings(TString &str1, TString &str2, TString &str3){
				 TString tmp;
				 tmp.append(str1);
				 tmp.append(str2);
				 tmp.append(str3);

				 return tmp;
			 }
	private: TString SetDouleSlash(TString &str){
				 TString s = str;
				 auto it = find(s.begin(), s.end(), '\\');
				 while (it != s.end()) {
					 auto it2 = s.insert(it, '\\');
					 it = find(it2+2, s.end(), '\\');
				 }
				 return s;
			 }
	private: System::Void btnNameSpaceDelete_Click(System::Object^  sender, System::EventArgs^  e) {
				 int iRecCount = dgvNameSpace->Rows->Count;

				 if (iRecCount <= 1) return void();

				 int iCurRow = dgvNameSpace->CurrentCell->RowIndex;

				 if ((iCurRow+1) == iRecCount) return void();

				 dgvNameSpace->Rows->RemoveAt(dgvNameSpace->CurrentCell->RowIndex);
			 }
	private: System::Void btnRun_Click(System::Object^  sender, System::EventArgs^  e) {

				 int iCountDB = dgvDBPath->Rows->Count;
				 if (iCountDB <= 1){ 
					 /*Выберите файлы БД*/
					// return void();
				 }

				 int iCountNS = dgvNameSpace->Rows->Count;
				 if (iCountNS <= 1){ 
					 /*Выберите файлы ПИ*/
					 //return void();
				 }

				 int THREAD_COUNT = 1;
				 // создание массива пустых указателей на потоки, которые создадим дальше
				 array<HANDLE>^ Thread_Arr = gcnew array<HANDLE>(THREAD_COUNT);


				 /*Path to NameSpace directory*/
				 TString sNSDirPath = AddStrings(sCurrentDir, TString(L"\\GS_APP"));
				 /*sNSDirPath.append(sCurrentDir);
				 sNSDirPath.append(L"\\GS_APP");*/

				 if (DirectoryExists((sNSDirPath))){
					 if (DeleteDirectory(sNSDirPath)){
					 }
					 else{

						 TString m = GetLastErrorDescription();
						 return void(); 
					 }
				 }


				 int k=0;
				 if (CreateDirectory(sNSDirPath.c_str(), NULL)){
					 k = 4;
				 }
				 else{
					 k = 5;
				 };


				 /*Create YAML file****************************************/

				 TString sNSFileName = AddStrings(sNSDirPath, TString(L"\\GS.Temp.yml"));
				 //sNSFileName.append(sNSDirPath);
				 //sNSFileName.append(L"\\GS.Temp.yml");

				 stringstream MS;

				 MS << "Uses: " << endl;

				 const char* ptr; 
				 TString sFileName, sFullFilePath;
				 string sTmp;
				 for (int i = 0; i < iCountNS-1; i++){				

					 TString sFileName = msclr::interop::marshal_as<TString>((String^)dgvNameSpace->Rows[i]->Cells[1]->Value);
					 TString sFullFilePath = msclr::interop::marshal_as<TString>((String^)dgvNameSpace->Rows[i]->Cells[2]->Value);

					 if (!copyFile(sFullFilePath.c_str(), AddStrings(AddStrings(sNSDirPath, TString(L"\\")), sFileName).c_str())){
						 //GetLastErrorDescription()
						 //return void();
					 }


					 ptr = (const char*)(Marshal::StringToHGlobalAnsi((String^)dgvNameSpace->Rows[i]->Cells[1]->Value)).ToPointer();
					 string sTmp = string(ptr);

					 MS << "  - \"" + GetRUIDNSFile(sFullFilePath) + " " + sTmp + "\"" << endl;


					 TString err = GetLastErrorDescription();
					 if (err != TString()){
						 int g= 2;
					 }
				 }

				 ofstream NSFile (sNSFileName.c_str(), ios::out | ios::binary);
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

				 //GetLastErrorDescription();

				 NSFile.close();
				 /*********************************************************/

				 ForThread *SendPr;

				 iCountDB = 2;
				 for (int i = 0; i < iCountDB-1; i++){	
					 SendPr = new ForThread;


					 TString s = TString();
					 s = AddStrings(quote, msclr::interop::marshal_as<TString>((String^)dgvDBPath->Rows[i]->Cells[1]->Value), quote);

					 basic_ostringstream<TCHAR> out;

					 out << AddStrings(quote, sGedeminPath, quote).c_str()
						 << " /sn " << s.c_str()
						 << " /user Administrator"
						 << " /password Administrator";
					 //<< " /q /SP "
					 /*<<  in->NSPath
					 << " /sfn "
					 << in->NSFileName*/
					 //<< endl;
					// TString s = TString();
					// s = msclr::interop::marshal_as<TString>((String^)dgvDBPath->Rows[i]->Cells[1]->Value);
					 //SendPr->NSFileName = AddStrings(quote, SetDouleSlash(sNSFileName), quote).c_str();
					 //SendPr->NSPath = sNSDirPath.c_str();//AddStrings(quote, SetDouleSlash(sNSDirPath), quote).c_str();
		
					// SendPr->DBPath = s.c_str();//AddStrings(quote, SetDouleSlash(TString(msclr::interop::marshal_as<TString>((String^)dgvDBPath->Rows[i]->Cells[1]->Value))), quote).c_str();
					 //s = out.str();
					 //SendPr->GedeminPath = s.c_str();//AddStrings(quote, SetDouleSlash(sGedeminPath), quote).c_str();

					 //SendPr->GedeminPath = L"calc";
					 SendPr->str = out.str();

					 Thread_Arr[i] =  CreateThread(0, 0, MyThread, SendPr, 0, 0);		
				 }
			 }
	private: System::Void Form1_Shown(System::Object^  sender, System::EventArgs^  e) {

				 TCHAR buffer[MAX_PATH];
				 GetModuleFileName( NULL, (LPWSTR)buffer, MAX_PATH );
				 string::size_type pos = TString( buffer ).find_last_of(L"\\/");
				 sCurrentDir = TString( buffer ).substr( 0, pos);

				 TString searchDBPath = L"LastDBPath="; 
				 TString searchNSPath = L"LastNSPath="; 
				 TString line;

				 wifstream SettingFile;

				 null_wcodecvt wcodec(1);
				 std::locale wloc(std::locale::classic(), &wcodec);
				 SettingFile.imbue(wloc);

				 SettingFile.open(AddStrings(sCurrentDir, TString(L"\\Settings.ini")), ios::app|ios::binary);
				 //SettingFile.imbue(locale(SettingFile.getloc(), new codecvt_utf8_utf16<char>));

				 while(getline(SettingFile, line)){ 
					 pos = line.find(searchDBPath, 0);
					 if (pos != string::npos) {			
						 sLastDBPath = line.substr(pos + searchDBPath.size(), line.size());
						 break;
					 }
				 }
				 while(getline(SettingFile, line)){ 
					 pos = line.find(searchNSPath, 0);
					 if (pos != string::npos) {			
						 sLastNSPath = line.substr(pos + searchNSPath.size(), line.size());
						 break;
					 }
				 }

				 SettingFile.close();

				 Application::DoEvents();
				 SearchOnAllDrives(L"gedemin.exe");

				 if (sGedeminPath != TString()){
					 tbGedeminPath->Text = gcnew String(sGedeminPath.c_str());
					 tbGedeminPath->BackColor = System::Drawing::Color::PaleGreen;
				 }
				 else{
					 tbGedeminPath->Text = "Не найден";
					 tbGedeminPath->BackColor = System::Drawing::Color::LightCoral;
				 }
			 }
	private: System::Void btnSearch_Click(System::Object^  sender, System::EventArgs^  e) {

				 sGedeminPath = TString();
				 tbGedeminPath->Text = "Поиск gedemin.exe...";
				 tbGedeminPath->BackColor = System::Drawing::SystemColors::Window;

				 Application::DoEvents();
				 SearchOnAllDrives(L"gedemin.exe");

				 if (sGedeminPath != TString()){
					 tbGedeminPath->Text = gcnew String(sGedeminPath.c_str());
					 tbGedeminPath->BackColor = System::Drawing::Color::PaleGreen;
				 }
				 else{
					 tbGedeminPath->Text = "Не найден";
					 tbGedeminPath->BackColor = System::Drawing::Color::LightCoral;
				 }
			 }
	};
}

