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
