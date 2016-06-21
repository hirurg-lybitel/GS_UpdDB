#pragma once
#include <iostream>
#include <windows.h>
using namespace std;

#ifndef UNICODE  
typedef string TString;
#else
typedef wstring TString;
#endif

/*VOID _func(OUT TString& strOUT, TString &str1, TString &str2) {
	TString tmp;
	tmp.append(str1);
	tmp.append(str2);

	strOUT = tmp;
}*/
/*VOID _AddStrings(const TCHAR* str1, TString &str2, OUT TString& strOUT) {
	TString tmp;
	tmp.append(TString(str1));
	tmp.append(str2);

	strOUT = tmp;
}
VOID AddStrings(TString &str1, const TCHAR* str2, OUT TString& strOUT) {
	TString tmp;
	tmp.append(str1);
	tmp.append(TString(str2));

	strOUT = tmp;
}
VOID AddStrings(TString &str1, TString &str2, TString &str3, OUT TString& strOUT) {
	TString tmp;
	tmp.append(str1);
	tmp.append(str2);
	tmp.append(str3);

	strOUT = tmp;
}*/

namespace GS_DataBaseUpdate {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	public ref class frmOptions : public System::Windows::Forms::Form {

		
		//	private: GS_DataBaseUpdate::frmMain^ OwnerForm2;

	public:
		frmOptions(void) {
			InitializeComponent();
		}
		/*frmOptions(System::Windows::Forms::Form ^ frm){
			OwnerForm = frm;
			InitializeComponent();

		}*/


		//private: System::Windows::Forms::Form^ OwnerForm;

	protected:
		/// <summary>
		/// Освободить все используемые ресурсы.
		/// </summary>
		~frmOptions() {
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::Button^  btnOk;
	protected:
	private: System::Windows::Forms::Button^  btnCancel;
	private: System::Windows::Forms::GroupBox^  gbPath;
	public: System::Windows::Forms::TextBox^  tbGedeminPath;
	public: System::Windows::Forms::TextBox^  tbSettingsPath;
	private: System::Windows::Forms::Label^  lbGedeminPath;
	private: System::Windows::Forms::Label^  lbLastNSPath;
	private: System::Windows::Forms::Label^  lbLastDBPath;
	private: System::Windows::Forms::Label^  lbSettingsPath;
	private: System::Windows::Forms::GroupBox^  gbOptions;

	public: System::Windows::Forms::TextBox^  tbLastNSPath;
	public: System::Windows::Forms::TextBox^  tbLastDBPath;
	public: System::Windows::Forms::CheckBox^  cbSearchGedemin;

	private:
		/// <summary>
		/// Обязательная переменная конструктора.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Требуемый метод для поддержки конструктора — не изменяйте 
		/// содержимое этого метода с помощью редактора кода.
		/// </summary>
		void InitializeComponent(void) {
			this->btnOk = (gcnew System::Windows::Forms::Button());
			this->btnCancel = (gcnew System::Windows::Forms::Button());
			this->gbPath = (gcnew System::Windows::Forms::GroupBox());
			this->tbLastNSPath = (gcnew System::Windows::Forms::TextBox());
			this->tbLastDBPath = (gcnew System::Windows::Forms::TextBox());
			this->tbGedeminPath = (gcnew System::Windows::Forms::TextBox());
			this->tbSettingsPath = (gcnew System::Windows::Forms::TextBox());
			this->lbGedeminPath = (gcnew System::Windows::Forms::Label());
			this->lbLastNSPath = (gcnew System::Windows::Forms::Label());
			this->lbLastDBPath = (gcnew System::Windows::Forms::Label());
			this->lbSettingsPath = (gcnew System::Windows::Forms::Label());
			this->gbOptions = (gcnew System::Windows::Forms::GroupBox());
			this->cbSearchGedemin = (gcnew System::Windows::Forms::CheckBox());
			this->gbPath->SuspendLayout();
			this->gbOptions->SuspendLayout();
			this->SuspendLayout();
			// 
			// btnOk
			// 
			this->btnOk->DialogResult = System::Windows::Forms::DialogResult::OK;
			this->btnOk->Location = System::Drawing::Point(383, 301);
			this->btnOk->Name = L"btnOk";
			this->btnOk->Size = System::Drawing::Size(87, 27);
			this->btnOk->TabIndex = 0;
			this->btnOk->Text = L"ОК";
			this->btnOk->UseVisualStyleBackColor = true;
			// 
			// btnCancel
			// 
			this->btnCancel->DialogResult = System::Windows::Forms::DialogResult::Cancel;
			this->btnCancel->Location = System::Drawing::Point(474, 301);
			this->btnCancel->Name = L"btnCancel";
			this->btnCancel->Size = System::Drawing::Size(87, 27);
			this->btnCancel->TabIndex = 1;
			this->btnCancel->Text = L"Отмена";
			this->btnCancel->UseVisualStyleBackColor = true;
			// 
			// gbPath
			// 
			this->gbPath->Controls->Add(this->tbLastNSPath);
			this->gbPath->Controls->Add(this->tbLastDBPath);
			this->gbPath->Controls->Add(this->tbGedeminPath);
			this->gbPath->Controls->Add(this->tbSettingsPath);
			this->gbPath->Controls->Add(this->lbGedeminPath);
			this->gbPath->Controls->Add(this->lbLastNSPath);
			this->gbPath->Controls->Add(this->lbLastDBPath);
			this->gbPath->Controls->Add(this->lbSettingsPath);
			this->gbPath->Location = System::Drawing::Point(12, 12);
			this->gbPath->Name = L"gbPath";
			this->gbPath->Size = System::Drawing::Size(549, 224);
			this->gbPath->TabIndex = 2;
			this->gbPath->TabStop = false;
			this->gbPath->Text = L"Системные";
			// 
			// tbLastNSPath
			// 
			this->tbLastNSPath->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(204)));
			this->tbLastNSPath->Location = System::Drawing::Point(18, 189);
			this->tbLastNSPath->Name = L"tbLastNSPath";
			this->tbLastNSPath->Size = System::Drawing::Size(517, 23);
			this->tbLastNSPath->TabIndex = 7;
			this->tbLastNSPath->Text = L"Ошибка чтения Settings.ini";
			// 
			// tbLastDBPath
			// 
			this->tbLastDBPath->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(204)));
			this->tbLastDBPath->Location = System::Drawing::Point(18, 141);
			this->tbLastDBPath->Name = L"tbLastDBPath";
			this->tbLastDBPath->Size = System::Drawing::Size(517, 23);
			this->tbLastDBPath->TabIndex = 6;
			this->tbLastDBPath->Text = L"Ошибка чтения Settings.ini";
			// 
			// tbGedeminPath
			// 
			this->tbGedeminPath->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(204)));
			this->tbGedeminPath->Location = System::Drawing::Point(18, 90);
			this->tbGedeminPath->Name = L"tbGedeminPath";
			this->tbGedeminPath->Size = System::Drawing::Size(517, 23);
			this->tbGedeminPath->TabIndex = 5;
			this->tbGedeminPath->Text = L"Ошибка чтения Settings.ini";
			// 
			// tbSettingsPath
			// 
			this->tbSettingsPath->BackColor = System::Drawing::SystemColors::Window;
			this->tbSettingsPath->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(204)));
			this->tbSettingsPath->Location = System::Drawing::Point(18, 41);
			this->tbSettingsPath->Name = L"tbSettingsPath";
			this->tbSettingsPath->ReadOnly = true;
			this->tbSettingsPath->Size = System::Drawing::Size(517, 23);
			this->tbSettingsPath->TabIndex = 4;
			this->tbSettingsPath->Text = L"Settings.ini не найден";
			// 
			// lbGedeminPath
			// 
			this->lbGedeminPath->AutoSize = true;
			this->lbGedeminPath->Location = System::Drawing::Point(15, 74);
			this->lbGedeminPath->Name = L"lbGedeminPath";
			this->lbGedeminPath->Size = System::Drawing::Size(106, 13);
			this->lbGedeminPath->TabIndex = 3;
			this->lbGedeminPath->Text = L"Путь к gedemin.exe:";
			// 
			// lbLastNSPath
			// 
			this->lbLastNSPath->AutoSize = true;
			this->lbLastNSPath->Location = System::Drawing::Point(15, 173);
			this->lbLastNSPath->Name = L"lbLastNSPath";
			this->lbLastNSPath->Size = System::Drawing::Size(128, 13);
			this->lbLastNSPath->TabIndex = 2;
			this->lbLastNSPath->Text = L"Последний каталог ПИ:";
			// 
			// lbLastDBPath
			// 
			this->lbLastDBPath->AutoSize = true;
			this->lbLastDBPath->Location = System::Drawing::Point(15, 125);
			this->lbLastDBPath->Name = L"lbLastDBPath";
			this->lbLastDBPath->Size = System::Drawing::Size(128, 13);
			this->lbLastDBPath->TabIndex = 1;
			this->lbLastDBPath->Text = L"Последний каталог БД:";
			// 
			// lbSettingsPath
			// 
			this->lbSettingsPath->AutoSize = true;
			this->lbSettingsPath->Location = System::Drawing::Point(15, 25);
			this->lbSettingsPath->Name = L"lbSettingsPath";
			this->lbSettingsPath->Size = System::Drawing::Size(127, 13);
			this->lbSettingsPath->TabIndex = 0;
			this->lbSettingsPath->Text = L"Путь к файлу настроек:";
			// 
			// gbOptions
			// 
			this->gbOptions->Controls->Add(this->cbSearchGedemin);
			this->gbOptions->Location = System::Drawing::Point(12, 242);
			this->gbOptions->Name = L"gbOptions";
			this->gbOptions->Size = System::Drawing::Size(549, 47);
			this->gbOptions->TabIndex = 4;
			this->gbOptions->TabStop = false;
			this->gbOptions->Text = L"Настройки";
			// 
			// cbSearchGedemin
			// 
			this->cbSearchGedemin->AutoSize = true;
			this->cbSearchGedemin->Location = System::Drawing::Point(18, 19);
			this->cbSearchGedemin->Name = L"cbSearchGedemin";
			this->cbSearchGedemin->Size = System::Drawing::Size(332, 17);
			this->cbSearchGedemin->TabIndex = 0;
			this->cbSearchGedemin->Text = L"Автоматически искать gedemin.exe при запуске программы";
			this->cbSearchGedemin->UseVisualStyleBackColor = true;
			// 
			// frmOptions
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(573, 340);
			this->Controls->Add(this->gbOptions);
			this->Controls->Add(this->gbPath);
			this->Controls->Add(this->btnCancel);
			this->Controls->Add(this->btnOk);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
			this->HelpButton = true;
			this->MaximizeBox = false;
			this->MinimizeBox = false;
			this->Name = L"frmOptions";
			this->ShowIcon = false;
			this->ShowInTaskbar = false;
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
			this->Text = L"Параметры";
			this->TopMost = true;
			this->Shown += gcnew System::EventHandler(this, &frmOptions::frmOptions_Shown);
			this->gbPath->ResumeLayout(false);
			this->gbPath->PerformLayout();
			this->gbOptions->ResumeLayout(false);
			this->gbOptions->PerformLayout();
			this->ResumeLayout(false);

		}
#pragma endregion

		/*private: TString AddStrings(TString &str1, TString &str2) {
			TString tmp;
			tmp.append(str1);
			tmp.append(str2);

			return tmp;
		}
		private: TString AddStrings(const TCHAR* str1, TString &str2) {
			TString tmp;
			tmp.append(TString(str1));
			tmp.append(str2);

			return tmp;
		}
		private: TString AddStrings(TString &str1, const TCHAR* str2) {
			TString tmp;
			tmp.append(str1);
			tmp.append(TString(str2));

			return tmp;
		}
		private: TString AddStrings(TString &str1, TString &str2, TString &str3) {
			TString tmp;
			tmp.append(str1);
			tmp.append(str2);
			tmp.append(str3);

			return tmp;
		}*/

	private: System::Void frmOptions_Shown(System::Object^  sender, System::EventArgs^  e) {
		

		//tbGedeminPath->Text = gcnew String(GS_DataBaseUpdate::frmMain::sGedeminPath.c_str());
		//GS_DataBaseUpdate::frmMain::AddStrings(L"", TString(L""));

		TCHAR buffer[MAX_PATH];

		GetModuleFileName(NULL, (LPWSTR)buffer, MAX_PATH);
		string::size_type pos = TString(buffer).find_last_of(L"\\/");
		TString sCurDir = TString(buffer).substr(0, pos);

		DWORD result;

		TString SettingPath;
		SettingPath.append(sCurDir);
		SettingPath.append(L"\\Settings.ini");

		tbSettingsPath->Text = gcnew String(SettingPath.c_str());

		//tbGedeminPath->Text = gcnew String(sGedeminPath.c_str());

		result = GetPrivateProfileString(
			TEXT("PathSection"),
			TEXT("LastDBPath"),
			NULL,
			buffer,
			MAX_PATH,
			SettingPath.c_str());
		if (result > 0)
			tbLastDBPath->Text = gcnew String(buffer);

		result = GetPrivateProfileString(
			TEXT("PathSection"),
			TEXT("LastNSPath"),
			NULL,
			buffer,
			MAX_PATH,
			SettingPath.c_str());
		if (result > 0)
			tbLastNSPath->Text = gcnew String(buffer);

		result = GetPrivateProfileString(
			TEXT("Options"),
			TEXT("SearchGedeminAfterOpen"),
			NULL,
			buffer,
			MAX_PATH,
			SettingPath.c_str());
		if (result > 0)
			cbSearchGedemin->Checked = (wcscmp(buffer, L"1") == 0 ? TRUE : FALSE);

		result = GetPrivateProfileString(
			TEXT("PathSection"),
			TEXT("GedeminPath"),
			NULL,
			buffer,
			MAX_PATH,
			SettingPath.c_str());
		if (result > 0)
			tbGedeminPath->Text = gcnew String(buffer);

	}
	};
}
