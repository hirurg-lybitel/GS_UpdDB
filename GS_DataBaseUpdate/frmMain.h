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

using namespace std;

#ifndef UNICODE  
typedef string TString;
#else
typedef wstring TString;
#endif

#define quote TString(L"\"")

/*Global Settings********************/
TString sGedeminPath = TString();
TString sCurrentDir = TString();;
TString sLastDBPath = TString();
TString sLastNSPath = TString();
TString searchDBPath = L"LastDBPath";
TString searchNSPath = L"LastNSPath";
/************************************/

struct ForThread {
	TString str;
};

HWND g_HWND;
BOOL CALLBACK EnumWindowsProcMy(HWND hwnd, LPARAM lParam) {
	DWORD lpdwProcessId;
	GetWindowThreadProcessId(hwnd, &lpdwProcessId);
	if (lpdwProcessId == lParam) {
		g_HWND = hwnd;
		return FALSE;
	}
	return TRUE;
}

BOOL DirectoryExists(const TString& sNSDirPath) {
	DWORD fa = GetFileAttributes(sNSDirPath.c_str());

	if (fa == INVALID_FILE_ATTRIBUTES) {
		return false;
	}

	if (fa & FILE_ATTRIBUTE_DIRECTORY) {
		return true;
	}

	return false;
}

TString GetLastErrorDescription(DWORD errCode = 0) {

	DWORD errorMessageID;
	if (errCode == 0) {
		errorMessageID = GetLastError();
		if (errorMessageID == 0)
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

BOOL DeleteDirectory(const TString& sDir, bool bDeleteSubDir = true) {

	bool bWasSubDir = false;
	HANDLE hDir;
	TString sFilePath;
	TString sPattern;
	WIN32_FIND_DATA FileInformation;

	sPattern.append(sDir);
	sPattern.append(L"\\*.*");

	hDir = FindFirstFile(sPattern.c_str(), &FileInformation);
	if (hDir != INVALID_HANDLE_VALUE) {
		do {
			if (FileInformation.cFileName[0] != '.') {
				sFilePath.erase();

				sFilePath.clear();
				sFilePath.append(sDir);
				sFilePath.append(L"\\");
				sFilePath.append(FileInformation.cFileName);

				if (FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					if (bDeleteSubDir) {
						// Delete subdirectory
						if (!DeleteDirectory(sFilePath, bDeleteSubDir))
							return false;
					}
					else
						bWasSubDir = true;
				}
				else {
					if (SetFileAttributes(sFilePath.c_str(), FILE_ATTRIBUTE_NORMAL) == FALSE)
						return false;

					if (DeleteFile(sFilePath.c_str()) == FALSE) return false;
				}
			}
		} while (FindNextFile(hDir, &FileInformation) == TRUE);

		//ERROR_NO_MORE_FILES
		if (!FindClose(hDir)) {
			return false;
		}
		else {
			if (!bWasSubDir) {
				if (SetFileAttributes(sDir.c_str(), FILE_ATTRIBUTE_NORMAL) == FALSE)
					return false;

				if (RemoveDirectory(sDir.c_str()) == FALSE)
					return false;
			}
		}
	}

	return true;
}

string IntToHex(int num) {
	ostringstream stream;
	stream << hex << num;
	return stream.str();
}

string GetMD5(stringstream& MS) {
	HCRYPTPROV hProv;
	HCRYPTHASH Hash;
	BYTE HashValue[16];
	DWORD HashValueSize;
	string HashString;

	if (!CryptAcquireContext(&hProv, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) return string();
	if (!CryptCreateHash(hProv, CALG_MD5, 0, 0, &Hash)) return string();

	HashValueSize = sizeof(HashValue);

	MS.seekg(0, ios::end);
	if (CryptHashData(Hash, (BYTE*)MS.str().c_str(), MS.tellg(), 0) && CryptGetHashParam(Hash, HP_HASHVAL, HashValue, &HashValueSize, 0)) {
		for (int i = 0; i < HashValueSize - 1; i++) {
			HashString += IntToHex(HashValue[i]);
		}
	}
	else return string();
	if (!CryptDestroyHash(Hash)) return string();
	if (!CryptReleaseContext(hProv, 0)) return string();

	transform(HashString.begin(), HashString.end(), HashString.begin(), toupper);
	return HashString;
}

string GetRUIDNSFile(TString& FileName) {
	string search = "RUID";
	string result = string();
	string line;

	ifstream fileInput;
	fileInput.open(FileName.c_str());
	string::size_type pos;

	while (getline(fileInput, line)) {
		pos = line.find(search, 0);
		if (pos != string::npos) {
			result = line.substr(pos + 6, line.size());
			break;
		}
	}
	return result;
}

BOOL copyFile(const TCHAR* cFrom, const TCHAR* cTo) {
	ifstream src(cFrom, ios::binary);
	ofstream dest(cTo, ios::binary);
	dest << src.rdbuf();
	return src && dest;
}

VOID ProcessFoundFile(LPWSTR CONST szPath, WIN32_FIND_DATA CONST * CONST fdFindData, LPWSTR CONST lpSearch) {
	//TCHAR szEnd[] = L"\r\n";
	//DWORD dwTemp;
	if (wcscmp(fdFindData->cFileName, lpSearch) == 0) {
		//WriteConsole(hStdOut, szPath, lstrlen(szPath), &dwTemp, NULL);
		//WriteConsole(hStdOut, szEnd, STRLEN(szEnd), &dwTemp, NULL);
		sGedeminPath = TString(szPath);
		//return TString(szPath);
	}
	//return TString();
}

VOID RecursiveSearch(LPWSTR szPath, LPWSTR CONST lpSearch, CONST BOOL OnlyFirst) {
	WIN32_FIND_DATA fdFindData;
	HANDLE hFind;
	TCHAR * CONST lpLastChar = szPath + lstrlen(szPath);

	lstrcat(szPath, L"*");
	hFind = FindFirstFile(szPath, &fdFindData);
	*lpLastChar = '\0';

	if (INVALID_HANDLE_VALUE == hFind) {
		//FindFirstFileFailed(szPath);
		return;
	}

	do {
		if ((0 == lstrcmp(fdFindData.cFileName, L".")) ||
			(0 == lstrcmp(fdFindData.cFileName, L".."))) {
			continue;
		}
		lstrcat(szPath, fdFindData.cFileName);
		if (fdFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			lstrcat(szPath, L"\\");
			RecursiveSearch(szPath, lpSearch, OnlyFirst);
		}
		else {
			ProcessFoundFile(szPath, &fdFindData, lpSearch);
		}
		*lpLastChar = '\0';

		if (OnlyFirst && sGedeminPath != TString()) return void();

	} while (FindNextFile(hFind, &fdFindData));

	FindClose(hFind);
}

VOID SearchOnAllDrives(LPWSTR CONST lpSearch, CONST BOOL OnlyFirst = true) {
	TCHAR szCurrDrive[] = L"A:\\";
	TCHAR szPath[MAX_PATH * 2 + 1];
	DWORD i, dwDisksMask = GetLogicalDrives();

	int N = 0;
	for (i = 0; i < 26; i++) {
		if (dwDisksMask & 1) {
			lstrcpy(szPath, szCurrDrive);
			RecursiveSearch(szPath, lpSearch, OnlyFirst);
			if (OnlyFirst && sGedeminPath != TString()) return void();
		}
		dwDisksMask >>= 1;
		szCurrDrive[0]++;
	}
}

string ConvertToString(TString& wStr) {
	int max_len = wStr.size();
	string str;
	str.resize(max_len);
	WideCharToMultiByte(CP_ACP, 0, wStr.c_str(), -1, (LPSTR)str.c_str(), max_len, NULL, NULL);
	return str;
}

DWORD WINAPI MyThread(LPVOID p) {

	ForThread* in = reinterpret_cast<ForThread*>(p);
	TString str = in->str;

	TCHAR tempCmdLine[MAX_PATH * 2];
	_tcscpy_s(tempCmdLine, MAX_PATH * 2, str.c_str());

	STARTUPINFO si = { sizeof(si) };
	PROCESS_INFORMATION pi;
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;

	if (CreateProcess(NULL, tempCmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
		WaitForSingleObject(pi.hProcess, INFINITE);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	else {
		TString mes = GetLastErrorDescription();
	}

	delete in;
	return 0;
}

namespace GS_DataBaseUpdate {

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

	/// <summary>
	/// Ñâîäêà äëÿ frmMain
	/// </summary>
	public ref class frmMain : public System::Windows::Forms::Form {
	public:
		frmMain(void) {
			InitializeComponent();
		}

	protected:
		~frmMain() {
			if (components) {
				delete components;
			}
		}

	private: System::Windows::Forms::OpenFileDialog^  ofdDBPath;
	private: System::Windows::Forms::OpenFileDialog^  ofdNameSpace;
	private: System::Windows::Forms::OpenFileDialog^  ofdGedeminPath;
	private: System::Windows::Forms::ToolStrip^  toolStrip1;
	private: System::Windows::Forms::ToolStripDropDownButton^  toolStripDropDownButton1;
	private: System::Windows::Forms::ToolStripMenuItem^  tsmiExit;
	private: System::Windows::Forms::ToolStripDropDownButton^  toolStripDropDownButton2;
	private: System::Windows::Forms::ToolStripMenuItem^  ïàðàìåòðûToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  îÏðîãðàììåToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  îáíîâëåíèåToolStripMenuItem;
	private: System::Windows::Forms::ToolStripButton^  tsbRun;
	private: System::Windows::Forms::GroupBox^  gbMain;
	private: System::Windows::Forms::Button^  btnNameSpaceDelete;
	private: System::Windows::Forms::Button^  btnNameSpaceAdd;
	private: System::Windows::Forms::Button^  btnDBPathDelete;
	private: System::Windows::Forms::Button^  btnDBPathAdd;
	private: System::Windows::Forms::DataGridView^  dgvNameSpace;
	private: System::Windows::Forms::DataGridView^  dgvDBPath;
	private: System::Windows::Forms::TextBox^  tbGedeminPath;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column1;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Column2;
	private: System::Windows::Forms::Button^  btnChooseEXE;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Number;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  NameNS;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  NameSpacePath;
	private: System::Windows::Forms::GroupBox^  gbStatus;
	private: System::Windows::Forms::DataGridView^  dgvEventLog;





	private: System::Windows::Forms::StatusStrip^  statusStrip2;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Description;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  isError;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Time;
	private: System::Windows::Forms::DataGridViewTextBoxColumn^  Date;









	protected:

	private:
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Òðåáóåìûé ìåòîä äëÿ ïîääåðæêè êîíñòðóêòîðà — íå èçìåíÿéòå 
		/// ñîäåðæèìîå ýòîãî ìåòîäà ñ ïîìîùüþ ðåäàêòîðà êîäà.
		/// </summary>
		void InitializeComponent(void) {
			System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(frmMain::typeid));
			System::Windows::Forms::DataGridViewCellStyle^  dataGridViewCellStyle1 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
			System::Windows::Forms::DataGridViewCellStyle^  dataGridViewCellStyle2 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
			System::Windows::Forms::DataGridViewCellStyle^  dataGridViewCellStyle3 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
			System::Windows::Forms::DataGridViewCellStyle^  dataGridViewCellStyle4 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
			this->ofdDBPath = (gcnew System::Windows::Forms::OpenFileDialog());
			this->ofdNameSpace = (gcnew System::Windows::Forms::OpenFileDialog());
			this->ofdGedeminPath = (gcnew System::Windows::Forms::OpenFileDialog());
			this->toolStrip1 = (gcnew System::Windows::Forms::ToolStrip());
			this->toolStripDropDownButton1 = (gcnew System::Windows::Forms::ToolStripDropDownButton());
			this->tsmiExit = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->toolStripDropDownButton2 = (gcnew System::Windows::Forms::ToolStripDropDownButton());
			this->ïàðàìåòðûToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->îÏðîãðàììåToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->îáíîâëåíèåToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->tsbRun = (gcnew System::Windows::Forms::ToolStripButton());
			this->gbMain = (gcnew System::Windows::Forms::GroupBox());
			this->btnChooseEXE = (gcnew System::Windows::Forms::Button());
			this->btnNameSpaceDelete = (gcnew System::Windows::Forms::Button());
			this->btnNameSpaceAdd = (gcnew System::Windows::Forms::Button());
			this->btnDBPathDelete = (gcnew System::Windows::Forms::Button());
			this->btnDBPathAdd = (gcnew System::Windows::Forms::Button());
			this->dgvNameSpace = (gcnew System::Windows::Forms::DataGridView());
			this->Number = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->NameNS = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->NameSpacePath = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->dgvDBPath = (gcnew System::Windows::Forms::DataGridView());
			this->Column1 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Column2 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->tbGedeminPath = (gcnew System::Windows::Forms::TextBox());
			this->gbStatus = (gcnew System::Windows::Forms::GroupBox());
			this->dgvEventLog = (gcnew System::Windows::Forms::DataGridView());
			this->Description = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->isError = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Time = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->Date = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
			this->statusStrip2 = (gcnew System::Windows::Forms::StatusStrip());
			this->toolStrip1->SuspendLayout();
			this->gbMain->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->dgvNameSpace))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->dgvDBPath))->BeginInit();
			this->gbStatus->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->dgvEventLog))->BeginInit();
			this->SuspendLayout();
			// 
			// ofdGedeminPath
			// 
			this->ofdGedeminPath->FileName = L"gedemin.exe";
			// 
			// toolStrip1
			// 
			this->toolStrip1->BackColor = System::Drawing::SystemColors::Control;
			this->toolStrip1->GripStyle = System::Windows::Forms::ToolStripGripStyle::Hidden;
			this->toolStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(3) {
				this->toolStripDropDownButton1,
					this->toolStripDropDownButton2, this->tsbRun
			});
			this->toolStrip1->Location = System::Drawing::Point(0, 0);
			this->toolStrip1->Name = L"toolStrip1";
			this->toolStrip1->RenderMode = System::Windows::Forms::ToolStripRenderMode::System;
			this->toolStrip1->Size = System::Drawing::Size(881, 25);
			this->toolStrip1->TabIndex = 16;
			this->toolStrip1->Text = L"toolStrip1";
			// 
			// toolStripDropDownButton1
			// 
			this->toolStripDropDownButton1->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Text;
			this->toolStripDropDownButton1->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(1) { this->tsmiExit });
			this->toolStripDropDownButton1->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"toolStripDropDownButton1.Image")));
			this->toolStripDropDownButton1->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->toolStripDropDownButton1->Name = L"toolStripDropDownButton1";
			this->toolStripDropDownButton1->Size = System::Drawing::Size(49, 22);
			this->toolStripDropDownButton1->Text = L"Ôàéë";
			// 
			// tsmiExit
			// 
			this->tsmiExit->Name = L"tsmiExit";
			this->tsmiExit->Size = System::Drawing::Size(108, 22);
			this->tsmiExit->Text = L"Âûõîä";
			this->tsmiExit->Click += gcnew System::EventHandler(this, &frmMain::tsmiExit_Click);
			// 
			// toolStripDropDownButton2
			// 
			this->toolStripDropDownButton2->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Text;
			this->toolStripDropDownButton2->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(3) {
				this->ïàðàìåòðûToolStripMenuItem,
					this->îÏðîãðàììåToolStripMenuItem, this->îáíîâëåíèåToolStripMenuItem
			});
			this->toolStripDropDownButton2->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"toolStripDropDownButton2.Image")));
			this->toolStripDropDownButton2->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->toolStripDropDownButton2->Name = L"toolStripDropDownButton2";
			this->toolStripDropDownButton2->Size = System::Drawing::Size(60, 22);
			this->toolStripDropDownButton2->Text = L"Ñåðâèñ";
			// 
			// ïàðàìåòðûToolStripMenuItem
			// 
			this->ïàðàìåòðûToolStripMenuItem->Name = L"ïàðàìåòðûToolStripMenuItem";
			this->ïàðàìåòðûToolStripMenuItem->Size = System::Drawing::Size(149, 22);
			this->ïàðàìåòðûToolStripMenuItem->Text = L"Ïàðàìåòðû";
			// 
			// îÏðîãðàììåToolStripMenuItem
			// 
			this->îÏðîãðàììåToolStripMenuItem->Name = L"îÏðîãðàììåToolStripMenuItem";
			this->îÏðîãðàììåToolStripMenuItem->Size = System::Drawing::Size(149, 22);
			this->îÏðîãðàììåToolStripMenuItem->Text = L"Î ïðîãðàììå";
			// 
			// îáíîâëåíèåToolStripMenuItem
			// 
			this->îáíîâëåíèåToolStripMenuItem->Name = L"îáíîâëåíèåToolStripMenuItem";
			this->îáíîâëåíèåToolStripMenuItem->Size = System::Drawing::Size(149, 22);
			this->îáíîâëåíèåToolStripMenuItem->Text = L"Îáíîâëåíèå";
			// 
			// tsbRun
			// 
			this->tsbRun->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"tsbRun.Image")));
			this->tsbRun->ImageTransparentColor = System::Drawing::Color::Magenta;
			this->tsbRun->Name = L"tsbRun";
			this->tsbRun->Size = System::Drawing::Size(82, 22);
			this->tsbRun->Text = L"Çàïóñòèòü";
			this->tsbRun->Click += gcnew System::EventHandler(this, &frmMain::tsbRun_Click);
			// 
			// gbMain
			// 
			this->gbMain->BackColor = System::Drawing::SystemColors::Control;
			this->gbMain->BackgroundImageLayout = System::Windows::Forms::ImageLayout::None;
			this->gbMain->Controls->Add(this->btnChooseEXE);
			this->gbMain->Controls->Add(this->btnNameSpaceDelete);
			this->gbMain->Controls->Add(this->btnNameSpaceAdd);
			this->gbMain->Controls->Add(this->btnDBPathDelete);
			this->gbMain->Controls->Add(this->btnDBPathAdd);
			this->gbMain->Controls->Add(this->dgvNameSpace);
			this->gbMain->Controls->Add(this->dgvDBPath);
			this->gbMain->Controls->Add(this->tbGedeminPath);
			this->gbMain->Location = System::Drawing::Point(12, 28);
			this->gbMain->Name = L"gbMain";
			this->gbMain->Size = System::Drawing::Size(615, 447);
			this->gbMain->TabIndex = 18;
			this->gbMain->TabStop = false;
			// 
			// btnChooseEXE
			// 
			this->btnChooseEXE->Location = System::Drawing::Point(505, 23);
			this->btnChooseEXE->Name = L"btnChooseEXE";
			this->btnChooseEXE->Size = System::Drawing::Size(75, 23);
			this->btnChooseEXE->TabIndex = 27;
			this->btnChooseEXE->Text = L"button1";
			this->btnChooseEXE->UseVisualStyleBackColor = true;
			this->btnChooseEXE->Click += gcnew System::EventHandler(this, &frmMain::btnChooseEXE_Click);
			// 
			// btnNameSpaceDelete
			// 
			this->btnNameSpaceDelete->Location = System::Drawing::Point(91, 246);
			this->btnNameSpaceDelete->Name = L"btnNameSpaceDelete";
			this->btnNameSpaceDelete->Size = System::Drawing::Size(73, 30);
			this->btnNameSpaceDelete->TabIndex = 25;
			this->btnNameSpaceDelete->Text = L"button5";
			this->btnNameSpaceDelete->UseVisualStyleBackColor = true;
			this->btnNameSpaceDelete->Click += gcnew System::EventHandler(this, &frmMain::btnNameSpaceDelete_Click);
			// 
			// btnNameSpaceAdd
			// 
			this->btnNameSpaceAdd->Location = System::Drawing::Point(12, 246);
			this->btnNameSpaceAdd->Name = L"btnNameSpaceAdd";
			this->btnNameSpaceAdd->Size = System::Drawing::Size(73, 30);
			this->btnNameSpaceAdd->TabIndex = 24;
			this->btnNameSpaceAdd->Text = L"button4";
			this->btnNameSpaceAdd->UseVisualStyleBackColor = true;
			this->btnNameSpaceAdd->Click += gcnew System::EventHandler(this, &frmMain::btnNameSpaceAdd_Click);
			// 
			// btnDBPathDelete
			// 
			this->btnDBPathDelete->Location = System::Drawing::Point(91, 51);
			this->btnDBPathDelete->Name = L"btnDBPathDelete";
			this->btnDBPathDelete->Size = System::Drawing::Size(73, 30);
			this->btnDBPathDelete->TabIndex = 23;
			this->btnDBPathDelete->Text = L"button3";
			this->btnDBPathDelete->UseVisualStyleBackColor = true;
			this->btnDBPathDelete->Click += gcnew System::EventHandler(this, &frmMain::btnDBPathDelete_Click);
			// 
			// btnDBPathAdd
			// 
			this->btnDBPathAdd->Location = System::Drawing::Point(12, 51);
			this->btnDBPathAdd->Name = L"btnDBPathAdd";
			this->btnDBPathAdd->Size = System::Drawing::Size(73, 30);
			this->btnDBPathAdd->TabIndex = 22;
			this->btnDBPathAdd->Text = L"button2";
			this->btnDBPathAdd->UseVisualStyleBackColor = true;
			this->btnDBPathAdd->Click += gcnew System::EventHandler(this, &frmMain::btnDBPathAdd_Click);
			// 
			// dgvNameSpace
			// 
			dataGridViewCellStyle1->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleLeft;
			dataGridViewCellStyle1->BackColor = System::Drawing::SystemColors::Control;
			dataGridViewCellStyle1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(204)));
			dataGridViewCellStyle1->ForeColor = System::Drawing::SystemColors::WindowText;
			dataGridViewCellStyle1->Padding = System::Windows::Forms::Padding(0, 3, 0, 3);
			dataGridViewCellStyle1->SelectionBackColor = System::Drawing::SystemColors::Highlight;
			dataGridViewCellStyle1->SelectionForeColor = System::Drawing::SystemColors::HighlightText;
			dataGridViewCellStyle1->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
			this->dgvNameSpace->ColumnHeadersDefaultCellStyle = dataGridViewCellStyle1;
			this->dgvNameSpace->ColumnHeadersHeightSizeMode = System::Windows::Forms::DataGridViewColumnHeadersHeightSizeMode::AutoSize;
			this->dgvNameSpace->Columns->AddRange(gcnew cli::array< System::Windows::Forms::DataGridViewColumn^  >(3) {
				this->Number,
					this->NameNS, this->NameSpacePath
			});
			this->dgvNameSpace->EnableHeadersVisualStyles = false;
			this->dgvNameSpace->Location = System::Drawing::Point(12, 282);
			this->dgvNameSpace->Name = L"dgvNameSpace";
			this->dgvNameSpace->RowHeadersVisible = false;
			this->dgvNameSpace->SelectionMode = System::Windows::Forms::DataGridViewSelectionMode::FullRowSelect;
			this->dgvNameSpace->Size = System::Drawing::Size(589, 150);
			this->dgvNameSpace->TabIndex = 20;
			// 
			// Number
			// 
			this->Number->Frozen = true;
			this->Number->HeaderText = L"¹";
			this->Number->Name = L"Number";
			this->Number->ReadOnly = true;
			this->Number->Resizable = System::Windows::Forms::DataGridViewTriState::False;
			this->Number->Width = 30;
			// 
			// NameNS
			// 
			this->NameNS->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::Fill;
			this->NameNS->HeaderText = L"Èìÿ ôàéëà ÏÈ";
			this->NameNS->Name = L"NameNS";
			this->NameNS->ReadOnly = true;
			// 
			// NameSpacePath
			// 
			this->NameSpacePath->HeaderText = L"NameSpacePath";
			this->NameSpacePath->Name = L"NameSpacePath";
			this->NameSpacePath->Visible = false;
			// 
			// dgvDBPath
			// 
			dataGridViewCellStyle2->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleLeft;
			dataGridViewCellStyle2->BackColor = System::Drawing::SystemColors::Control;
			dataGridViewCellStyle2->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(204)));
			dataGridViewCellStyle2->ForeColor = System::Drawing::SystemColors::WindowText;
			dataGridViewCellStyle2->Padding = System::Windows::Forms::Padding(0, 3, 0, 3);
			dataGridViewCellStyle2->SelectionBackColor = System::Drawing::SystemColors::Highlight;
			dataGridViewCellStyle2->SelectionForeColor = System::Drawing::SystemColors::HighlightText;
			dataGridViewCellStyle2->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
			this->dgvDBPath->ColumnHeadersDefaultCellStyle = dataGridViewCellStyle2;
			this->dgvDBPath->ColumnHeadersHeightSizeMode = System::Windows::Forms::DataGridViewColumnHeadersHeightSizeMode::AutoSize;
			this->dgvDBPath->Columns->AddRange(gcnew cli::array< System::Windows::Forms::DataGridViewColumn^  >(2) { this->Column1, this->Column2 });
			this->dgvDBPath->EnableHeadersVisualStyles = false;
			this->dgvDBPath->Location = System::Drawing::Point(12, 87);
			this->dgvDBPath->Name = L"dgvDBPath";
			this->dgvDBPath->RowHeadersVisible = false;
			this->dgvDBPath->SelectionMode = System::Windows::Forms::DataGridViewSelectionMode::FullRowSelect;
			this->dgvDBPath->Size = System::Drawing::Size(589, 150);
			this->dgvDBPath->TabIndex = 19;
			// 
			// Column1
			// 
			this->Column1->Frozen = true;
			this->Column1->HeaderText = L"¹";
			this->Column1->Name = L"Column1";
			this->Column1->Resizable = System::Windows::Forms::DataGridViewTriState::False;
			this->Column1->Width = 30;
			// 
			// Column2
			// 
			this->Column2->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::Fill;
			this->Column2->HeaderText = L"Ïóòü ê áàçå äàííûõ";
			this->Column2->Name = L"Column2";
			// 
			// tbGedeminPath
			// 
			this->tbGedeminPath->BackColor = System::Drawing::SystemColors::Window;
			this->tbGedeminPath->Cursor = System::Windows::Forms::Cursors::IBeam;
			this->tbGedeminPath->Enabled = false;
			this->tbGedeminPath->Font = (gcnew System::Drawing::Font(L"Times New Roman", 15, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Bold | System::Drawing::FontStyle::Italic)),
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(204)));
			this->tbGedeminPath->Location = System::Drawing::Point(12, 19);
			this->tbGedeminPath->Name = L"tbGedeminPath";
			this->tbGedeminPath->Size = System::Drawing::Size(487, 30);
			this->tbGedeminPath->TabIndex = 18;
			this->tbGedeminPath->Text = L"Ïîèñê gedemin.exe...";
			// 
			// gbStatus
			// 
			this->gbStatus->Controls->Add(this->dgvEventLog);
			this->gbStatus->Location = System::Drawing::Point(633, 28);
			this->gbStatus->Name = L"gbStatus";
			this->gbStatus->Size = System::Drawing::Size(236, 447);
			this->gbStatus->TabIndex = 19;
			this->gbStatus->TabStop = false;
			// 
			// dgvEventLog
			// 
			this->dgvEventLog->AllowUserToAddRows = false;
			this->dgvEventLog->AllowUserToDeleteRows = false;
			this->dgvEventLog->AllowUserToResizeColumns = false;
			this->dgvEventLog->AllowUserToResizeRows = false;
			this->dgvEventLog->AutoSizeColumnsMode = System::Windows::Forms::DataGridViewAutoSizeColumnsMode::Fill;
			this->dgvEventLog->AutoSizeRowsMode = System::Windows::Forms::DataGridViewAutoSizeRowsMode::AllCells;
			this->dgvEventLog->BackgroundColor = System::Drawing::SystemColors::Window;
			this->dgvEventLog->CellBorderStyle = System::Windows::Forms::DataGridViewCellBorderStyle::None;
			dataGridViewCellStyle3->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
			dataGridViewCellStyle3->BackColor = System::Drawing::SystemColors::Control;
			dataGridViewCellStyle3->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(204)));
			dataGridViewCellStyle3->ForeColor = System::Drawing::SystemColors::WindowText;
			dataGridViewCellStyle3->SelectionBackColor = System::Drawing::SystemColors::Highlight;
			dataGridViewCellStyle3->SelectionForeColor = System::Drawing::SystemColors::HighlightText;
			dataGridViewCellStyle3->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
			this->dgvEventLog->ColumnHeadersDefaultCellStyle = dataGridViewCellStyle3;
			this->dgvEventLog->ColumnHeadersHeightSizeMode = System::Windows::Forms::DataGridViewColumnHeadersHeightSizeMode::AutoSize;
			this->dgvEventLog->Columns->AddRange(gcnew cli::array< System::Windows::Forms::DataGridViewColumn^  >(4) {
				this->Description,
					this->isError, this->Time, this->Date
			});
			dataGridViewCellStyle4->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleLeft;
			dataGridViewCellStyle4->BackColor = System::Drawing::SystemColors::Window;
			dataGridViewCellStyle4->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 11, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(204)));
			dataGridViewCellStyle4->ForeColor = System::Drawing::SystemColors::ControlText;
			dataGridViewCellStyle4->SelectionBackColor = System::Drawing::SystemColors::Highlight;
			dataGridViewCellStyle4->SelectionForeColor = System::Drawing::SystemColors::HighlightText;
			dataGridViewCellStyle4->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
			this->dgvEventLog->DefaultCellStyle = dataGridViewCellStyle4;
			this->dgvEventLog->Location = System::Drawing::Point(6, 19);
			this->dgvEventLog->MultiSelect = false;
			this->dgvEventLog->Name = L"dgvEventLog";
			this->dgvEventLog->ReadOnly = true;
			this->dgvEventLog->RowHeadersVisible = false;
			this->dgvEventLog->RowTemplate->Resizable = System::Windows::Forms::DataGridViewTriState::True;
			this->dgvEventLog->ScrollBars = System::Windows::Forms::ScrollBars::Vertical;
			this->dgvEventLog->SelectionMode = System::Windows::Forms::DataGridViewSelectionMode::FullRowSelect;
			this->dgvEventLog->Size = System::Drawing::Size(224, 413);
			this->dgvEventLog->TabIndex = 0;
			this->dgvEventLog->RowsAdded += gcnew System::Windows::Forms::DataGridViewRowsAddedEventHandler(this, &frmMain::dgvEventLog_RowsAdded);
			// 
			// Description
			// 
			this->Description->HeaderText = L"Ëîã ñîáûòèé";
			this->Description->Name = L"Description";
			this->Description->ReadOnly = true;
			this->Description->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
			// 
			// isError
			// 
			this->isError->HeaderText = L"Îøèáêà";
			this->isError->Name = L"isError";
			this->isError->ReadOnly = true;
			this->isError->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
			this->isError->Visible = false;
			// 
			// Time
			// 
			this->Time->HeaderText = L"Âðåìÿ";
			this->Time->Name = L"Time";
			this->Time->ReadOnly = true;
			this->Time->Visible = false;
			// 
			// Date
			// 
			this->Date->HeaderText = L"Äàòà";
			this->Date->Name = L"Date";
			this->Date->ReadOnly = true;
			this->Date->Visible = false;
			// 
			// statusStrip2
			// 
			this->statusStrip2->Location = System::Drawing::Point(0, 486);
			this->statusStrip2->Name = L"statusStrip2";
			this->statusStrip2->Size = System::Drawing::Size(881, 22);
			this->statusStrip2->TabIndex = 20;
			this->statusStrip2->Text = L"statusStrip2";
			// 
			// frmMain
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(881, 508);
			this->Controls->Add(this->statusStrip2);
			this->Controls->Add(this->gbStatus);
			this->Controls->Add(this->toolStrip1);
			this->Controls->Add(this->gbMain);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedSingle;
			this->Icon = (cli::safe_cast<System::Drawing::Icon^>(resources->GetObject(L"$this.Icon")));
			this->MaximizeBox = false;
			this->Name = L"frmMain";
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
			this->Text = L"Îáíîâëåíèå ÁÄ";
			this->FormClosing += gcnew System::Windows::Forms::FormClosingEventHandler(this, &frmMain::frmMain_FormClosing);
			this->Shown += gcnew System::EventHandler(this, &frmMain::frmMain_Shown);
			this->toolStrip1->ResumeLayout(false);
			this->toolStrip1->PerformLayout();
			this->gbMain->ResumeLayout(false);
			this->gbMain->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->dgvNameSpace))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->dgvDBPath))->EndInit();
			this->gbStatus->ResumeLayout(false);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->dgvEventLog))->EndInit();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion


	private: TString AddStrings(TString &str1, TString &str2) {
		TString tmp;
		tmp.append(str1);
		tmp.append(str2);

		return tmp;
	}
	private: TString AddStrings(TString &str1, TString &str2, TString &str3) {
		TString tmp;
		tmp.append(str1);
		tmp.append(str2);
		tmp.append(str3);

		return tmp;
	}
	private: System::Void AddToEventLog(TString str, BOOL isError, BOOL withMsgBox) {

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
			MessageBox::Show(gcnew String(str.c_str()), "Âíèìàíèå!", MessageBoxButtons::OK, MessageBoxIcon::Error);
		}

		//delete[] bufTime;
		//delete[] bufDate;
	}
	private: TString SetDouleSlash(TString &str) {
		TString s = str;
		auto it = find(s.begin(), s.end(), '\\');
		while (it != s.end()) {
			auto it2 = s.insert(it, '\\');
			it = find(it2 + 2, s.end(), '\\');
		}
		return s;
	}
	private: System::Void btnDBPathAdd_Click(System::Object^  sender, System::EventArgs^  e) {
		ofdDBPath->Title = "Âûáåðèòå áàçó äàííûõ";
		ofdDBPath->Filter = "Ôàéëû áàçû äàííûõ (*.FDB)|*.FDB|Âñå ôàéëû (*.*)|*.*";
		ofdDBPath->FileName = "";
		ofdDBPath->InitialDirectory = gcnew String(sLastDBPath.c_str());

		if (ofdDBPath->ShowDialog() == System::Windows::Forms::DialogResult::OK) {

			TString tmp = msclr::interop::marshal_as<TString>((String^)ofdDBPath->FileName);
			string::size_type pos = tmp.find_last_of(L"\\/");
			sLastDBPath = tmp.substr(0, pos);

			DWORD res;
			res = WritePrivateProfileString(
				TEXT("PathSection"),
				searchDBPath.c_str(),
				sLastDBPath.c_str(),
				AddStrings(sCurrentDir, TString(L"\\Settings.ini")).c_str());

			dgvDBPath->Rows->Add(dgvDBPath->Rows->Count, ofdDBPath->FileName);			
		}
	}
	private: System::Void btnDBPathDelete_Click(System::Object^  sender, System::EventArgs^  e) {

		int iRecCount = dgvDBPath->Rows->Count;

		if (iRecCount <= 1) return void();

		int iCurRow = dgvDBPath->CurrentCell->RowIndex;

		if ((iCurRow + 1) == iRecCount) return void();

		dgvDBPath->Rows->RemoveAt(dgvDBPath->CurrentCell->RowIndex);

		/*Delete all rows*/
		//while (iRecCount > 1) dgvDBPath->Rows->RemoveAt(0);
	}
	private: System::Void btnNameSpaceAdd_Click(System::Object^  sender, System::EventArgs^  e) {
		ofdNameSpace->Title = "Âûáåðèòå ôàéëû ÏÈ";
		ofdNameSpace->Filter = "Ôàéëû YML|*.yml";
		ofdNameSpace->FileName = "";
		ofdNameSpace->Multiselect = true;
		ofdNameSpace->InitialDirectory = gcnew String(sLastNSPath.c_str());

		if (ofdNameSpace->ShowDialog() == System::Windows::Forms::DialogResult::OK) {

			TString tmp = msclr::interop::marshal_as<TString>((String^)ofdNameSpace->FileName);
			string::size_type pos = tmp.find_last_of(L"\\/");
			sLastNSPath = tmp.substr(0, pos);

			DWORD res;
			res = WritePrivateProfileString(
				TEXT("PathSection"),
				searchNSPath.c_str(),
				sLastNSPath.c_str(),
				AddStrings(sCurrentDir, TString(L"\\Settings.ini")).c_str());

			int iFileCount = ofdNameSpace->SafeFileNames->Length;

			for (int i = 0; i < iFileCount; i++) {
				dgvNameSpace->Rows->Add(dgvNameSpace->Rows->Count, ofdNameSpace->SafeFileNames[i], ofdNameSpace->FileNames[i]);
			}
		}
	}
	private: System::Void btnNameSpaceDelete_Click(System::Object^  sender, System::EventArgs^  e) {

		int iRecCount = dgvNameSpace->Rows->Count;

		if (iRecCount <= 1) return void();

		int iCurRow = dgvNameSpace->CurrentCell->RowIndex;

		if ((iCurRow + 1) == iRecCount) return void();

		dgvNameSpace->Rows->RemoveAt(dgvNameSpace->CurrentCell->RowIndex);
	}
	private: System::Void frmMain_Shown(System::Object^  sender, System::EventArgs^  e) {

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

		result = GetPrivateProfileString(
			TEXT("PathSection"),
			searchNSPath.c_str(),
			NULL,
			buffer,
			MAX_PATH,
			AddStrings(sCurrentDir, TString(L"\\Settings.ini")).c_str());
		if (result > 0)
			sLastNSPath = TString(buffer);

		Application::DoEvents();
		//SearchOnAllDrives(L"gedemin.exe");

		if (sGedeminPath != TString()) {
			tbGedeminPath->Text = gcnew String(sGedeminPath.c_str());
			tbGedeminPath->BackColor = Color::PaleGreen;
		}
		else {
			tbGedeminPath->Text = "Íå íàéäåí";
			tbGedeminPath->BackColor = Color::LightCoral;
		}
	}
	private: System::Void tsbRun_Click(System::Object^  sender, System::EventArgs^  e) {

		AddToEventLog(L"asfafsfasfqwertyuiasdfghjzxcvbnm123456789", false, false);

		return;
		int iCountDB = dgvDBPath->Rows->Count;
		if (iCountDB <= 1) {
			/*Âûáåðèòå ôàéëû ÁÄ*/
			
			//AddToEventLog(L"lksdgflkjsd", 1, 1);
			return void();
		}

		int iCountNS = dgvNameSpace->Rows->Count;
		if (iCountNS <= 1) {
			/*Âûáåðèòå ôàéëû ÏÈ*/
			//return void();
		}

		int THREAD_COUNT = iCountDB - 1;
		// ñîçäàíèå ìàññèâà ïóñòûõ óêàçàòåëåé íà ïîòîêè, êîòîðûå ñîçäàäèì äàëüøå
		HANDLE* Thread_Arr = new HANDLE[THREAD_COUNT];

		/*Path to NameSpace directory*/
		TString sNSDirPath = AddStrings(sCurrentDir, TString(L"\\GS_APP"));

		if (DirectoryExists((sNSDirPath))) {
			if (DeleteDirectory(sNSDirPath)) {
			}
			else {

				TString m = GetLastErrorDescription();
				return void();
			}
		}


		int k = 0;
		if (CreateDirectory(sNSDirPath.c_str(), NULL)) {
			k = 4;
		}
		else {
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
		for (int i = 0; i < iCountNS - 1; i++) {

			TString sFileName = msclr::interop::marshal_as<TString>((String^)dgvNameSpace->Rows[i]->Cells[1]->Value);
			TString sFullFilePath = msclr::interop::marshal_as<TString>((String^)dgvNameSpace->Rows[i]->Cells[2]->Value);

			if (!copyFile(sFullFilePath.c_str(), AddStrings(AddStrings(sNSDirPath, TString(L"\\")), sFileName).c_str())) {
				//GetLastErrorDescription()
				//return void();
			}


			ptr = (const char*)(Marshal::StringToHGlobalAnsi((String^)dgvNameSpace->Rows[i]->Cells[1]->Value)).ToPointer();
			string sTmp = string(ptr);

			MS << "  - \"" + GetRUIDNSFile(sFullFilePath) + " " + sTmp + "\"" << endl;


			TString err = GetLastErrorDescription();
			if (err != TString()) {
				int g = 2;
			}
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

		//GetLastErrorDescription();

		NSFile.close();
		/*********************************************************/

		ForThread *SendPr;
		TString s;

		for (int i = 0; i < iCountDB - 1; i++) {
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

			Thread_Arr[i] = CreateThread(0, 0, MyThread, SendPr, 0, 0);
		}
	}
	private: System::Void btnChooseEXE_Click(System::Object^  sender, System::EventArgs^  e) {

		ofdGedeminPath->Title = "Âûáåðèòå ôàéë gedemin.exe";
		ofdGedeminPath->Filter = "Ôàéëû EXE|*.exe";
		ofdGedeminPath->FileName = "gedemin.exe";
		ofdNameSpace->RestoreDirectory = true;

		if (ofdNameSpace->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
			sGedeminPath = msclr::interop::marshal_as<TString>((String^)ofdNameSpace->FileName);

			tbGedeminPath->Text = gcnew String(sGedeminPath.c_str());
			tbGedeminPath->BackColor = Color::PaleGreen;


		}
	}
	private: System::Void tsmiExit_Click(System::Object^  sender, System::EventArgs^  e) {
		System::Windows::Forms::DialogResult Result;
		Result = MessageBox::Show("Âû òî÷íî õîòèòå âûéòè?", "Âíèìàíèå!", MessageBoxButtons::YesNo, MessageBoxIcon::Question);

		if (Result == System::Windows::Forms::DialogResult::Yes)
			Application::Exit();
	}
	private: System::Void frmMain_FormClosing(System::Object^  sender, System::Windows::Forms::FormClosingEventArgs^  e) {
		__super::OnClosing(e);
		if (e->CloseReason == System::Windows::Forms::CloseReason::ApplicationExitCall)
			Application::Exit();
		else {
			if (MessageBox::Show("Âû òî÷íî õîòèòå âûéòè?", "Âûõîä", MessageBoxButtons::YesNo,
				MessageBoxIcon::Question) == System::Windows::Forms::DialogResult::No)
				e->Cancel = true;
			else
				Application::Exit();
		}
	}
	private: System::Void dgvEventLog_RowsAdded(System::Object^  sender, System::Windows::Forms::DataGridViewRowsAddedEventArgs^  e) {
				
		dgvEventLog->Rows[e->RowIndex]->DefaultCellStyle->ForeColor = Color::Blue;

		if (Convert::ToInt16(dgvEventLog->Rows[e->RowIndex]->Cells[1]->Value->ToString()) == 1)
			dgvEventLog->Rows[e->RowIndex]->DefaultCellStyle->ForeColor = Color::Red;

	}
};
}