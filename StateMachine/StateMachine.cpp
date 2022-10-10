#include <windows.h>
#include <string>
#include <cstdlib>
#include <iostream>
#include <vector>

#include <commctrl.h>
#include <mysql.h>
#include <mysqld_error.h>
#include <dshow.h>

#define ID_MENU_TABLE 1000
#define ID_MENU_QUERY 1001
#define ID_MENU_EXIT 1002

#define ID_EDIT_TABLENAME 100
#define ID_LB_FIELDS 101
#define ID_BTN_OK 102
#define ID_LV_RESTABLE 104

#define ID_BTN_NEXT 105
#define ID_CB_OPERATIONS 106
#define ID_EDIT_VALUE 107

#define ID_ERROR 108
#define ID_SUCCESS 109

using namespace std;

enum states {
	start, state_1, state_2, state_3, state_4, state_5, state_6, state_7,
	state_8, state_9, state_10, state_11, state_12, finish
};

enum  actions {
	actUnknown, actClearEdit, actConnectDB, actShowError, actShowAllTable, actReturnTo1, actShowFields,
	actAddFields, actSelectOnlyFields, actAddWhere, actAddFieldWhere, actAddOperation, actAddValue, actSelectWhere,
	actAddLogical, actFinishProgram
};

enum signals {
	sigUnknown = 0, sigModeTable = 1, sigModeQuery = 2, sigBtnOk = 3, sigError = 4,
	sigSuccess = 5, sigBtnNext = 6, sigAddField = 7, sigAddOperation = 8, sigExitProgram = 9
};

//Таблица переходов
states changeState[14][10] = {
	//sigUnknown  sigModeTable sigModeQuery sigBtnOk	 sigError  sigSuccess  sigBtnNext  sigAddField	 sigAddOperation  sigExitProgram
	   { state_1, 	start, 	    start,        start, 	 start,    start,      start, 	    start,      start, 	          start },    //start
	   { state_1, 	state_2,    state_6,      state_1,  state_1,  state_1,    state_1,    state_1,    state_1,         finish },   //state_1
	   { state_2, 	state_2,    state_2,      state_3,  state_2,  state_2,    state_2,    state_2,    state_2,         state_2 },  //state_2
	   { state_3, 	state_3,    state_3,      state_3,  state_4,  state_5,    state_3,    state_3,    state_3,         state_3 },  //state_3
	   { state_4, 	state_4,    state_4,      state_2,  state_4,  state_4,    state_4,    state_4,    state_4,         state_4 },  //state_4
	   { state_5, 	state_5,    state_5,      state_1,  state_5,  state_5,    state_5,    state_5,    state_5,         state_5 },  //state_5
	   { state_6, 	state_6,    state_6,      state_6,  state_6,  state_6,    state_7,    state_6,    state_6,         state_6 },  //state_6
	   { state_7, 	state_7,    state_7,      state_8,  state_7,  state_7,    state_9,    state_7,    state_7,         state_7 },  //state_7
	   { state_8, 	state_8,    state_8,      state_1,  state_8,  state_8,    state_8,    state_8,    state_8,         state_8 },  //state_8
	   { state_9, 	state_9,    state_9,      state_9,  state_9,  state_9,    state_9,    state_10,   state_9,         state_9 },  //state_9
	   { state_10, state_10,  state_10,     state_10, state_10, state_10,   state_10,   state_10,   state_11,        state_10 }, //state_10
	   { state_11, state_11,  state_11,     state_11, state_11, state_11,   state_12,   state_11,   state_11,        state_11 }, //state_11
	   { state_12, state_12,  state_12,     state_8,  state_12, state_12,   state_9,    state_12,   state_12,        state_12 }, //state_12
	   { finish,   finish,    finish,       finish,   finish,   finish,     finish,     finish,     finish,          finish },   //finish
};

//Таблица состояний
actions getAction[14][10] = {
	//sigUnknown  sigModeTable  sigModeQuery   sigBtnOk		      sigError		sigSuccess		  sigBtnNext	  sigAddField		 sigAddOperation	 sigExitProgram
	{ actUnknown, actUnknown,   actUnknown,    actUnknown, 	      actUnknown, 	actUnknown,       actUnknown, 	  actUnknown, 	     actUnknown, 		 actUnknown },         //start
	{ actUnknown, actClearEdit, actClearEdit,  actUnknown, 	      actUnknown, 	actUnknown,       actUnknown, 	  actUnknown, 	     actUnknown, 		 actFinishProgram },   //state_1
	{ actUnknown, actUnknown,   actUnknown,    actConnectDB,        actUnknown, 	actUnknown,       actUnknown, 	  actUnknown, 	     actUnknown, 		 actUnknown },         //state_2
	{ actUnknown, actUnknown,   actUnknown,    actUnknown, 	      actShowError, actShowAllTable, actUnknown, 	  actUnknown, 	     actUnknown, 		 actUnknown },         //state_3
	{ actUnknown, actUnknown,   actUnknown,    actClearEdit,        actUnknown, 	actUnknown,       actUnknown, 	  actUnknown, 	     actUnknown, 		 actUnknown },         //state_4
	{ actUnknown, actUnknown,   actUnknown, 	 actReturnTo1,        actUnknown, 	actUnknown,       actUnknown, 	  actUnknown, 	     actUnknown, 		 actUnknown },         //state_5
	{ actUnknown, actUnknown,   actUnknown, 	 actUnknown, 	      actUnknown, 	actUnknown,       actShowFields, actUnknown, 	     actUnknown, 		 actUnknown },         //state_6
	{ actUnknown, actUnknown,   actUnknown, 	 actSelectOnlyFields, actUnknown, 	actUnknown,       actAddWhere, 	  actAddFields,      actUnknown, 		 actUnknown },         //state_7
	{ actUnknown, actUnknown,   actUnknown, 	 actReturnTo1,        actUnknown, 	actUnknown,       actUnknown, 	  actUnknown, 	     actUnknown, 		 actUnknown },         //state_8
	{ actUnknown, actUnknown,   actUnknown, 	 actUnknown, 	      actUnknown, 	actUnknown,       actUnknown, 	  actAddFieldWhere,  actUnknown, 		 actUnknown },         //state_9
	{ actUnknown, actUnknown,   actUnknown,     actUnknown, 	      actUnknown, 	actUnknown,       actUnknown, 	  actUnknown, 	     actAddOperation,   actUnknown },         //state_10
	{ actUnknown, actUnknown,   actUnknown, 	 actUnknown, 	      actUnknown, 	actUnknown,       actAddValue,   actUnknown, 	     actUnknown, 		 actUnknown },         //state_11
	{ actUnknown, actUnknown,   actUnknown, 	 actSelectWhere,      actUnknown, 	actUnknown,       actAddLogical, actUnknown, 	     actUnknown, 		 actUnknown },         //state_12
	{ actUnknown, actUnknown,   actUnknown, 	 actUnknown, 	      actUnknown, 	actUnknown,       actUnknown, 	  actUnknown, 	     actUnknown, 		 actUnknown },         //finish
};

//Таблица контекста состояний 
/*
	0 - Меню, 1 - таблица, 2 - список с полями таблицы, 3 - список со операциями,
	4 - текстовое поле для ввода таблицы, 5 - текст.поле для ввода значения, 6 - ОК,
	7 - Далее, 8 - многострочное текстовое поле
*/
string tableContext[14][9] = {
	// 0    1    2    3    4    5    6    7    8
	{ "+", " ", " ", " ", " ", " ", " ", " ", " " },    //start
	{ "+", " ", " ", " ", " ", " ", " ", " ", " " },    //state_1
	{ " ", " ", " ", " ", "+", " ", "+", " ", "+" },    //state_2
	{ " ", " ", " ", " ", "+", " ", "-", " ", "-" },    //state_3
	{ " ", " ", " ", " ", "-", " ", "+", " ", "-" },    //state_4
	{ " ", "+", " ", " ", "-", " ", "+", " ", "-" },    //state_5
	{ " ", " ", " ", " ", "+", " ", "-", "+", "+" },    //state_6
	{ " ", " ", "+", " ", "-", " ", "+", "+", "+" },    //state_7
	{ " ", "+", "-", " ", "-", " ", "+", "-", "-" },    //state_8
	{ " ", " ", "+", "-", "-", "-", "-", "-", "+" },    //state_9
	{ " ", " ", "-", "+", "-", "-", "-", "-", "+" },    //state_10
	{ " ", " ", "-", "-", "-", "+", "-", "+", "+" },    //state_11
	{ " ", " ", "-", "+", "-", " ", "+", "+", "+" },    //state_12
	{ " ", " ", " ", " ", " ", " ", " ", " ", " " },    //finish
};
enum states current_state = state_1;
enum signals current_signal = sigUnknown;
enum actions current_action;

HINSTANCE hInst;

BOOL RegClass(WNDPROC, LPCTSTR, UINT);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

HWND CreateListView(HWND, UINT);

void getSignal(int, int);
void changeContext(states);
void doQuery(string curQuery);

string errorMsg;
bool first = true;
int colCount;

HMENU hMainMenu, hModeMenu;
HWND hwndEditTableName;
HWND hwndListFields;

HWND hwndQueryStatic;
HWND hwndBtnOK;
HWND hwndBtnNext;

HWND hwndResTable;
HWND hwndOperations;
HWND hwndEditValue;

string operations[8] = { "=", ">", "<", ">=", "<=", "!=", "AND", "OR" };

MYSQL* conn, mysql;
MYSQL_RES* res;
MYSQL_ROW row;
MYSQL_FIELD* field;

char globalTableName[64];
string globalQuery;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		//region WM_CREATE
	case WM_CREATE:
	{
		mysql_init(&mysql);
		conn = mysql_real_connect(&mysql, "localhost", "root", "1234", "winapi", 3306, NULL, 0);

		hMainMenu = CreateMenu();
		hModeMenu = CreatePopupMenu();

		AppendMenu(hMainMenu, MF_STRING | MF_POPUP, (UINT_PTR)hModeMenu, "Mode");
		AppendMenu(hModeMenu, MF_STRING, ID_MENU_TABLE, "Table");
		AppendMenu(hModeMenu, MF_STRING, ID_MENU_QUERY, "Request");

		SetMenu(hWnd, hMainMenu);
		SetMenu(hWnd, hModeMenu);

		hwndEditTableName = CreateWindowEx(0, "EDIT", " ",
			WS_CHILD | ES_LEFT | WS_BORDER,
			0, 0, 200, 25,
			hWnd, (HMENU)ID_EDIT_TABLENAME, hInst, NULL);

		hwndListFields = CreateWindowEx(0, "listbox", NULL,
			WS_CHILD | LBS_STANDARD | CBS_AUTOHSCROLL | WS_VSCROLL,
			210, 0, 150, 200,
			hWnd, (HMENU)ID_LB_FIELDS, hInst, NULL);

		hwndQueryStatic = CreateWindowEx(0, "STATIC", NULL,
			WS_CHILD,
			0, 30, 200, 165,
			hWnd, nullptr, hInst, NULL);

		hwndBtnOK = CreateWindowEx(0, "button", "OK",
			WS_CHILD | BS_PUSHBUTTON,
			450, 10, 50, 25,
			hWnd, (HMENU)ID_BTN_OK, hInst, NULL);

		hwndBtnNext = CreateWindowEx(0, "button", "Next",
			WS_CHILD | BS_PUSHBUTTON,
			450, 45, 50, 25,
			hWnd, (HMENU)ID_BTN_NEXT, hInst, NULL);

		hwndResTable = CreateListView(hWnd, ID_LV_RESTABLE);

		hwndOperations = CreateWindowEx(WS_EX_STATICEDGE, "COMBOBOX", nullptr,
			CBS_DROPDOWNLIST | WS_CHILD | WS_VSCROLL,
			375, 80, 60, 500,
			hWnd, (HMENU)ID_CB_OPERATIONS, hInst, NULL);

		hwndEditValue = CreateWindowEx(0, "EDIT", "",
			WS_CHILD | ES_LEFT | WS_BORDER,
			375, 130, 200, 25,
			hWnd, (HMENU)ID_EDIT_VALUE, hInst, NULL);

		EnableWindow(hwndEditValue, FALSE);

		break;
	}
	//endregion

	case WM_COMMAND:
	{
		int lowordId = LOWORD(wParam);
		int hiwordId = HIWORD(wParam);

		getSignal(lowordId, hiwordId);
		current_action = getAction[current_state][current_signal];
		switch (current_action) {

			//region actConnectDB
		case actConnectDB:
		{
			if (conn)
			{
				globalQuery = "SELECT * FROM ";

				GetWindowText(hwndEditTableName, globalTableName, 64);
				globalQuery += globalTableName;

				const char* q = globalQuery.c_str();
				mysql_query(conn, q);

				if (res = mysql_store_result(conn))
				{
					PostMessage(hWnd, WM_COMMAND, ID_SUCCESS, hiwordId);
				}
				else
				{
					errorMsg = mysql_error(conn);
					PostMessage(hWnd, WM_COMMAND, ID_ERROR, hiwordId);
				}
			}
			else
			{
				cout << "ERR" << endl;
				PostMessage(hWnd, WM_COMMAND, ID_ERROR, hiwordId);
			}

			break;
		}
		//endregion

		//region actClearEdit
		case actClearEdit:
		{
			PostMessage(hwndEditTableName, EM_SETSEL, 0, -1);
			PostMessage(hwndEditTableName, WM_CLEAR, 0, 0);
			break;
		}
		//endregion

		//region actShowError
		case actShowError:
		{
			if (MessageBox(hWnd, errorMsg.c_str(), "Error", MB_OK | MB_ICONERROR) == IDOK)
				PostMessage(hWnd, WM_COMMAND, ID_BTN_OK, hiwordId);
			break;
		}
		//endregion

		//region actShowAllTable
		case actShowAllTable: //Вывод всей таблицы
		{
			globalQuery = "SELECT * FROM ";
			globalQuery += globalTableName;

			SetWindowText(hwndQueryStatic, globalQuery.c_str());

			doQuery(globalQuery);

			break;
		}
		//endregion

		//region actReturnTo1
		case actReturnTo1:
		{
			globalQuery = "";
			for (int i = colCount - 1; i >= 0; i--)
			{
				ListView_DeleteColumn(hwndResTable, i);
			}
			ListView_DeleteAllItems(hwndResTable);
			PostMessage(hwndListFields, LB_RESETCONTENT, 0, 0);
			Static_SetText(hwndQueryStatic, " ");
			break;
		}
		//endregion

		//region actShowFields
		case actShowFields:     //Вывод полей таблицы
		{
			string query = "SELECT * FROM ";

			GetWindowText(hwndEditTableName, globalTableName, 64);
			query += globalTableName;

			const char* q = query.c_str();
			mysql_query(conn, q);
			res = mysql_store_result(conn);

			while ((field = mysql_fetch_field(res)))
			{
				SendMessage(hwndListFields, LB_ADDSTRING, 0, (LPARAM)field->name);
			}
			globalQuery = "SELECT ";
			SetWindowText(hwndQueryStatic, globalQuery.c_str());
			PostMessage(hwndEditValue, EM_SETSEL, 0, -1);
			PostMessage(hwndEditValue, WM_CLEAR, 0, 0);

			break;
		}
		//endregion

		//region actAddFields
		case actAddFields:    //Добавление поля таблицы в запрос
		{
			int index = (int)SendMessage(hwndListFields, LB_GETCURSEL, 0, wParam);
			if (index != LB_ERR)
			{
				char fieldName[32];
				SendMessage(hwndListFields, LB_GETTEXT, (WPARAM)index, (LPARAM)fieldName);

				if (first)
				{
					first = !first;
				}
				else
				{
					globalQuery += ", ";
				}
				globalQuery += fieldName;
				SetWindowText(hwndQueryStatic, globalQuery.c_str());
			}

			break;
		}
		//endregion

		//region actSelectOnlyFields
		case actSelectOnlyFields:
		{
			globalQuery += " FROM ";

			globalQuery += globalTableName;
			SetWindowText(hwndQueryStatic, globalQuery.c_str());

			doQuery(globalQuery);
			break;
		}
		//endregion

		//region actAddWhere
		case actAddWhere:
		{
			globalQuery += " FROM ";

			globalQuery += globalTableName;
			globalQuery += " WHERE ";
			SetWindowText(hwndQueryStatic, globalQuery.c_str());
			first = true;
			break;
		}
		//endregion

		//region actAddFieldWhere
		case actAddFieldWhere:  //Выбор поля для WHERE
		{
			int fieldsIndex = (int)SendMessage(hwndListFields, LB_GETCURSEL, 0, wParam);
			if (fieldsIndex != LB_ERR)
			{
				char fieldName[10];
				SendMessage(hwndListFields, LB_GETTEXT, (WPARAM)fieldsIndex, (LPARAM)fieldName);

				globalQuery += fieldName;
				SetWindowText(hwndQueryStatic, globalQuery.c_str());
			}
			for (int i = 0; i < 6; i++)
			{
				ComboBox_AddString(hwndOperations, operations[i].c_str());
			}
		}
		//endregion

		//region actAddOperation
		case actAddOperation:  //Добавление операции сравнения в запрос
		{
			first = true;
			int operationIndex = SendMessage(hwndOperations, CB_GETCURSEL, 0, 0);
			if (operationIndex != CB_ERR)
			{
				globalQuery += operations[operationIndex];

				SetWindowText(hwndQueryStatic, globalQuery.c_str());
				ComboBox_ResetContent(hwndOperations);
			}

			break;
		}
		//endregion

		//region actAddValue
		case actAddValue:
		{
			char value[64];
			GetWindowText(hwndEditValue, value, 64);

			globalQuery += "'";
			globalQuery += value;
			globalQuery += "'";
			SetWindowText(hwndQueryStatic, globalQuery.c_str());
			for (int i = 6; i < 8; i++)
			{
				ComboBox_AddString(hwndOperations, operations[i].c_str());
			}
			break;
		}
		//endregion

		//region actSelectWhere
		case actSelectWhere:
		{
			ComboBox_ResetContent(hwndOperations);

			doQuery(globalQuery);
			break;
		}
		//endregion

		//region actAddLogical
		case actAddLogical:
		{
			int logicalIndex = SendMessage(hwndOperations, CB_GETCURSEL, 0, 0);
			if (logicalIndex != CB_ERR)
			{
				globalQuery += " ";
				globalQuery += operations[logicalIndex + 6];
				globalQuery += " ";
				SetWindowText(hwndQueryStatic, globalQuery.c_str());
				ComboBox_ResetContent(hwndOperations);
			}
			PostMessage(hwndEditValue, EM_SETSEL, 0, -1);
			PostMessage(hwndEditValue, WM_CLEAR, 0, 0);
			break;
		}
		//endregion

		//region actFinishProgram
		case actFinishProgram:
		{
			mysql_close(conn);
			PostQuitMessage(0);
			break;
		}
		//endregion

		default: break;
		}
		current_state = changeState[current_state][current_signal];
		changeContext(current_state);

	}
	break;
	case WM_DESTROY:

		PostQuitMessage(0);
		break;
	case WM_CLOSE:
		SendMessage(hWnd, WM_COMMAND, ID_MENU_EXIT, 0);
		break;
	default:

		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void doQuery(string curQuery)
{
	const char* q = curQuery.c_str();

	if (mysql_query(conn, q) != 0)
	{
		MessageBox(NULL, "Error: can't execute query", "Err", MB_OK | MB_ICONERROR);
		return;
	}
	res = mysql_store_result(conn);

	//Структура для создания столбцов
	LVCOLUMN structColumn;
	memset(&structColumn, 0, sizeof(structColumn));
	structColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
	structColumn.fmt = LVCFMT_LEFT;
	structColumn.cx = 100; //ширина столбца

	int i = 0;
	while (field = mysql_fetch_field(res))
	{
		structColumn.pszText = field->name;
		ListView_InsertColumn(hwndResTable, i, &structColumn);
		i++;
	}

	int lviRow = 0;
	colCount = mysql_num_fields(res);
	while (row = mysql_fetch_row(res))
	{
		LVITEM item;
		memset(&item, 0, sizeof(LVITEM));
		item.mask = LVIF_TEXT;
		item.iItem = lviRow;
		int list = ListView_InsertItem(hwndResTable, &item);
		for (int j = 0; j < colCount; j++)
		{
			item.iSubItem = j;
			ListView_InsertItem(hwndResTable, &item);
			ListView_SetItemText(hwndResTable, list, j, row[j]);
		}
		lviRow++;
	}

	first = true;
}

void getSignal(int lowordId, int hiwordId)
{
	switch (lowordId) {
	case ID_MENU_TABLE:
		current_signal = sigModeTable;
		break;
	case ID_MENU_QUERY:
		current_signal = sigModeQuery;
		break;
	case ID_BTN_OK:
		current_signal = sigBtnOk;
		break;
	case ID_BTN_NEXT:
		current_signal = sigBtnNext;
		if (current_state == state_12)
			if (SendMessage(hwndOperations, CB_GETCURSEL, 0, 0) != CB_ERR)
				current_signal = sigBtnNext;
			else current_signal = sigUnknown;

		break;
	case ID_ERROR:
		current_signal = sigError;
		break;
	case ID_SUCCESS:
		current_signal = sigSuccess;
		break;
	case ID_LB_FIELDS:
		switch (hiwordId) {
		case LBN_DBLCLK:
			current_signal = sigAddField;
			break;
		default:
			current_signal = sigUnknown;
		}
		break;
	case ID_CB_OPERATIONS:
		switch (hiwordId) {
		case CBN_SELCHANGE:
			current_signal = sigAddOperation;
			break;
		default:
			current_signal = sigUnknown;
		}
		break;

	case ID_MENU_EXIT:
		current_signal = sigExitProgram;
		break;

	default:
		current_signal = sigUnknown;
		break;
	}
}

void changeContext(states state)
{
	for (int i = 0; i < 9; i++)
		switch (i)
		{
		case 0:
			EnableMenuItem(hMainMenu, ID_MENU_TABLE, tableContext[state][i] == "+" ? MF_ENABLED : MF_DISABLED);
			EnableMenuItem(hMainMenu, ID_MENU_QUERY, tableContext[state][i] == "+" ? MF_ENABLED : MF_DISABLED);
			EnableMenuItem(hMainMenu, ID_MENU_EXIT, tableContext[state][i] == "+" ? MF_ENABLED : MF_DISABLED);
			break;
		case 1:
			ShowWindow(hwndResTable, tableContext[state][i] != " " ? SW_SHOW : SW_HIDE);
			EnableWindow(hwndResTable, tableContext[state][i] == "+" ? true : false);
			break;
		case 2:
			ShowWindow(hwndListFields, tableContext[state][i] != " " ? SW_SHOW : SW_HIDE);
			EnableWindow(hwndListFields, tableContext[state][i] == "+" ? true : false);
			break;
		case 3:
			ShowWindow(hwndOperations, tableContext[state][i] != " " ? SW_SHOW : SW_HIDE);
			EnableWindow(hwndOperations, tableContext[state][i] == "+" ? true : false);
			break;
		case 4:
			ShowWindow(hwndEditTableName, tableContext[state][i] != " " ? SW_SHOW : SW_HIDE);
			EnableWindow(hwndEditTableName, tableContext[state][i] == "+" ? true : false);
			break;
		case 5:
			ShowWindow(hwndEditValue, tableContext[state][i] != " " ? SW_SHOW : SW_HIDE);
			EnableWindow(hwndEditValue, tableContext[state][i] == "+" ? true : false);
			break;
		case 6:
			ShowWindow(hwndBtnOK, tableContext[state][i] != " " ? SW_SHOW : SW_HIDE);
			EnableWindow(hwndBtnOK, tableContext[state][i] == "+" ? true : false);
			break;
		case 7:
			ShowWindow(hwndBtnNext, tableContext[state][i] != " " ? SW_SHOW : SW_HIDE);
			EnableWindow(hwndBtnNext, tableContext[state][i] == "+" ? true : false);
			break;
		case 8:
			ShowWindow(hwndQueryStatic, tableContext[state][i] != " " ? SW_SHOW : SW_HIDE);
			EnableWindow(hwndQueryStatic, tableContext[state][i] == "+" ? true : false);
			break;
		default: break;
		}
	InvalidateRect(GetParent(hwndQueryStatic), NULL, TRUE);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;

	hInst = hInstance;

	if (!RegClass(WndProc, "Main", COLOR_WINDOW)) { return FALSE; };

	HWND hWnd = CreateWindow("Main", "Main",
		WS_VISIBLE | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT,
		600, 600,
		nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		MessageBox(NULL, "Window Creation Failed.", "Error", MB_OK | MB_ICONERROR);
		return FALSE;
	}
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}

BOOL RegClass(WNDPROC Proc, LPCTSTR szName, UINT brBackground)
{
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.lpfnWndProc = Proc;
	wc.hInstance = hInst;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(brBackground + 1);
	wc.lpszMenuName = (LPCTSTR)NULL;
	wc.lpszClassName = szName;
	return (RegisterClass(&wc) != 0);
}

HWND CreateListView(HWND hWndParent, UINT uId)
{
	INITCOMMONCONTROLSEX icex;
	icex.dwICC = ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&icex);

	RECT rcClient;
	GetClientRect(hWndParent, &rcClient);

	HWND hWndLV = CreateWindowEx(NULL, WC_LISTVIEW, NULL,
		WS_CHILD | WS_BORDER | LVS_REPORT | LVS_REPORT | LVS_EDITLABELS | LVS_EX_FULLROWSELECT,
		0, 250,
		600, 300,
		hWndParent, (HMENU)uId, hInst, NULL);

	ListView_SetExtendedListViewStyleEx(hWndLV, 0, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	return (hWndLV);
}