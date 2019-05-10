#include "WinMain.h"
#include "MC_KernelWork.h"
#include <iostream>
using namespace std;
int main()
{
	MC_KernelWork tKernelWork;
	tKernelWork.OnLoad();

	while(1)
	{
		tKernelWork.Upload();
		//system("pause");
		//Sleep(1000*60*10);
	}

	
	printf("\n ----------end---------- ");
	fflush(stdin);
	getchar();
	return 0;
}
//---------------------------------------------------------------------------