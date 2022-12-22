#include "framework.h"
#include "main.h"
#include "rrt.h"
#define MAX_LOADSTRING 100


HINSTANCE hInst;                               
WCHAR szTitle[MAX_LOADSTRING];                  
WCHAR szWindowClass[MAX_LOADSTRING];            

std::string save = "output.txt", load = "input.txt";

bool setRobot = 0, setObstacle = 1, setGoal = 0;
bool show_path = 0;
bool dragging = 0;
Circle* dragged;

int iterations = 500;
int right_border = 700, bottom_border = 700;
double robot_r = 4; //материальнная точка - робот
Circle robot;
int robot_color[3] = { 129, 2, 129 };
Point start = { 100, 100 }, goal = { 600, 600 };
std::vector<Circle> obstacles;
Graph path;
std::vector<Point> path_ver;
std::vector<Point> result;

void draw_line(Point a, Point b, HDC hdc) {
    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(57, 57, 57));

    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

    MoveToEx(hdc, a.x, a.y, NULL);
    LineTo(hdc, b.x, b.y);

    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
}

void draw_path(std::vector<Point> path, HDC hdc) {
    if (path.empty()) return;
    HPEN hPen = CreatePen(PS_SOLID, 3, RGB(0, 202, 202));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    MoveToEx(hdc, path[0].x, path[0].y, NULL);
    for (auto it = path.begin() + 1; it != path.end(); it++) {
        LineTo(hdc, (*it).x, (*it).y);
    }
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
}

void draw_circle(Circle c, HDC hdc) {
    Point o = c.get_point();
    double r = c.get_r();
    int* col = c.get_color();
    HBRUSH hBrush = CreateSolidBrush(RGB(col[0], col[1], col[2]));

    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
    Ellipse(hdc, o.x - r, o.y - r, o.x + r, o.y + r);

    SelectObject(hdc, hOldBrush);
    DeleteObject(hBrush);
}

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	Input(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_COOLPROGRAM, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_COOLPROGRAM));

    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}




ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_COOLPROGRAM));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_COOLPROGRAM);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; 

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)   
   {
      return FALSE;
   }

   robot.set_color(robot_color);
   robot.set_r(robot_r);
   robot.move_to(start.x, start.y);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE: {
        HMENU hMainMenu = CreateMenu();
        HMENU hFile = CreateMenu();
        HMENU hSet = CreateMenu();
        HMENU hSaveLoad = CreateMenu();

        AppendMenuW(hMainMenu, MF_POPUP, (UINT_PTR)hFile, L"Меню");
        AppendMenuW(hMainMenu, MF_POPUP, (UINT_PTR)hSet, L"Настройки сцены");
        AppendMenuW(hMainMenu, MF_POPUP, (UINT_PTR)hSaveLoad, L"Файл");

        AppendMenuW(hSet, MF_POPUP, ID_SET_ROB, L"Робот");
        AppendMenuW(hSet, MF_POPUP, ID_SET_OBS, L"Препятсвия");
        AppendMenuW(hSet, MF_POPUP, ID_SET_GOAL, L"Цель");

        AppendMenuW(hFile, MF_BYCOMMAND, ID_FIND_PATH, L"Найти путь");
        AppendMenuW(hFile, MF_BYCOMMAND, ID_SETTINGS, L"Настройки");
        AppendMenuW(hFile, MF_BYCOMMAND, ID_RESET, L"Очистить сцену");

        AppendMenuW(hSaveLoad, MF_BYCOMMAND, ID_SAVE, L"Сохранить");
        AppendMenuW(hSaveLoad, MF_BYCOMMAND, ID_LOAD, L"Загрузить");

        AppendMenuW(hFile, MF_BYCOMMAND, IDM_ABOUT, L"Справка");

        AppendMenuW(hFile, MF_BYCOMMAND, IDM_EXIT, L"Выход");

        SetMenu(hWnd, hMainMenu);
        break;
    }
	case WM_LBUTTONDOWN: {
        {
            show_path = 0;
            HDC hdc = GetDC(hWnd);
            SelectClipRgn(hdc, CreateRectRgn(0, 0, right_border + 1, bottom_border + 1));
            if (setObstacle) {
                bool moved = 0;
                for (auto it = obstacles.begin(); it != obstacles.end(); it++) {
                    Point cur = { LOWORD(lParam), HIWORD(lParam) };
                    Point o = (*it).get_point();
                    if (distance(cur, o) < (*it).get_r()) {
                        moved = 1;
                        dragging = 1;
                        dragged = &(*it);
                        break;
                    }
                }
                if (!moved) {
                    Circle new_obs;
                    new_obs.move_to(LOWORD(lParam), HIWORD(lParam));
                    obstacles.push_back(new_obs);
                }
            }
            if (setRobot) {
                robot.move_to(LOWORD(lParam), HIWORD(lParam));
                start = robot.get_point();
            }
            if (setGoal) {
                goal.x = LOWORD(lParam);
                goal.y = HIWORD(lParam);
            }
        }
        RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		return 0L;
	}
    case WM_LBUTTONUP: {
        if (dragging) {
            Point cur = { LOWORD(lParam), HIWORD(lParam) };
            (*dragged).move_to(cur.x, cur.y);
            dragging = 0;
            RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
        }
        break;
    }
    case WM_RBUTTONDOWN: {
        if (setObstacle) {
            for (auto it = obstacles.begin(); it != obstacles.end(); it++) {
                Point cur = { LOWORD(lParam), HIWORD(lParam) };
                Point o = (*it).get_point();
                if (distance(cur, o) < (*it).get_r()) {
                    show_path = 0;
                    obstacles.erase(it);
                    RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
                    break;
                }
            }
        }
    }
    case WM_MOUSEWHEEL: {
        if (setRobot) {
            POINT pt;
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
            ScreenToClient(hWnd, &pt);
            Point cur = { pt.x, pt.y };
            Point o = robot.get_point();
            if (distance(cur, o) < robot_r) {
                show_path = 0;
                auto mousewheel = GET_WHEEL_DELTA_WPARAM(wParam);
                if (mousewheel > 0) {
                    if (robot_r < 50) robot_r += 3;
                    if (robot_r > 50) robot_r = 50;
                }
                else {
                    if (robot_r > 3) robot_r -= 3;
                    if (robot_r < 3) robot_r = 3;
                }
                robot.set_r(robot_r);
                RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
                break;
            }
        }
        for (auto it = obstacles.begin(); it != obstacles.end(); it++) {
            POINT pt;
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
            ScreenToClient(hWnd, &pt);
            Point cur = { pt.x, pt.y };
            Point o = (*it).get_point();
            if (distance(cur, o) < (*it).get_r()) {
                show_path = 0;
                auto mousewheel = GET_WHEEL_DELTA_WPARAM(wParam);
                if (mousewheel > 0) {
                    if ((*it).get_r() < 200) (*it).set_r((*it).get_r() + 3);
                    if ((*it).get_r() > 200) (*it).set_r(200);
                }
                else {
                    if ((*it).get_r() > 5) (*it).set_r((*it).get_r() - 3);
                    if ((*it).get_r() < 5) (*it).set_r(5);
                }
                RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
                break;
            }
        }

    }
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case ID_SET_OBS:
            setObstacle = TRUE;
            setRobot = FALSE;
            setGoal = FALSE;
            break;
        case ID_SET_ROB:
            setRobot = TRUE;
            setObstacle = FALSE;
            setGoal = FALSE;
            break;
        case ID_SET_GOAL:
            setGoal = TRUE;
            setObstacle = FALSE;
            setRobot = FALSE;
            break;
        case ID_FIND_PATH:
            try {
                path = rrt(iterations, start, goal);
            }
            catch (...) {
                MessageBoxA(hWnd, "Некорректная стартовая позиция", "Ошибка", MB_OK);
            }
            path_ver = path.get_vertices();
            result = find_path(path, start, goal);
            show_path = 1;
            if (!result.empty()) {
                result.insert(result.begin(), start);
            }
            else {
                MessageBoxA(hWnd, "Путь не найден","Завершено", MB_OK);
            }
            RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
                break;
        case ID_SAVE: {
            std::ofstream fout(save);
            fout << right_border << " " << bottom_border << " " << iterations << std::endl;
            fout << robot.get_point().x << " " << robot.get_point().y << " " << robot.get_r() << std::endl;
            fout << goal.x << " " << goal.y << std::endl;
            fout << obstacles.size() << std::endl;
            for (Circle c : obstacles)
                fout << c.get_point().x << " " << c.get_point().y << " " << c.get_r() << std::endl;
            fout.close();
            InvalidateRect(hWnd, NULL, TRUE);
        }
            break;
        case ID_LOAD: {
            try {
                obstacles.clear();
                std::ifstream fin(load);
                if (!(fin >> right_border >> bottom_border >> iterations)) throw "Некорректный входной файл";
                double x, y, r;
                if (!(fin >> x >> y >> r)) throw "Некорректный входной файл";
                robot.move_to(x, y);
                robot.set_r(r);
                start = { x, y };
                if (!(fin >> goal.x >> goal.y)) throw "Некорректный входной файл";
                int n;
                if (!(fin >> n)) throw "Некорректный входной файл";
                for (int i = 0; i < n; i++) {
                    Circle temp;
                    if (!(fin >> x >> y >> r)) throw "Некорректный входной файл";
                    temp.move_to(x, y);
                    temp.set_r(r);
                    obstacles.push_back(temp);
                }
                fin.close();
                show_path = 0;
            }
            catch (...) {
                MessageBoxA(hWnd, "Некорректный входной файл", "Ошибка", MB_OK);
            }
            InvalidateRect(hWnd, NULL, TRUE);
        }
            break;
        case ID_RESET: {
            obstacles.clear();
            robot.move_to(robot_r, robot_r);
            start = robot.get_point();
            goal = { right_border - robot_r, bottom_border - robot_r };
            show_path = 0;
        }
                     break;
        case ID_SETTINGS:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_INPUT), hWnd, Input);
            break;
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
        break;
    }
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            RECT rt;
            GetClientRect(hWnd, &rt);

            SelectClipRgn(hdc, CreateRectRgn(0, 0, right_border + 1, bottom_border + 1));

            POINT border[4];
            border[0] = { 0, 0 };
            border[1] = { int(right_border), 0 };
            border[2] = { int(right_border), int(bottom_border) };
            border[3] = { 0, int(bottom_border) };
            Polygon(hdc, border, 4);
            draw_circle(robot, hdc);
            Circle g;
            g.move_to(goal.x, goal.y);
            g.set_r(5);
            int col[3] = { 0, 0, 255 };
            g.set_color(col);
            draw_circle(g, hdc);
            for (Circle& i : obstacles) draw_circle(i, hdc);
            if (show_path) {
                for (Point& p1 : path_ver)
                    for (Point& p2 : path.get_adjacent_vertices(p1))
                        draw_line(p1, p2, hdc);
                draw_path(result, hdc);
            }

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK Input(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG: {
        LPSTR l = const_cast<char*>(load.c_str());
        LPSTR s = const_cast<char*>(save.c_str());
        std::string it_str = std::to_string(iterations);
        std::string bb_str = std::to_string(bottom_border);
        std::string rb_str = std::to_string(right_border);
        LPSTR it = const_cast<char*>(it_str.c_str());
        LPSTR bb = const_cast<char*>(bb_str.c_str());
        LPSTR rb = const_cast<char*>(rb_str.c_str());
        SetDlgItemTextA(hDlg, IDC_EDIT1, bb);
        SetDlgItemTextA(hDlg, IDC_EDIT2, rb);
        SetDlgItemTextA(hDlg, IDC_EDIT4, it);
        SetDlgItemTextA(hDlg, IDC_EDIT5, l);
        SetDlgItemTextA(hDlg, IDC_EDIT6, s);
        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            if (LOWORD(wParam) == IDOK) {
                auto temp_border = GetDlgItemInt(hDlg, IDC_EDIT2, NULL, FALSE);
                if (temp_border) right_border = temp_border;
                temp_border = GetDlgItemInt(hDlg, IDC_EDIT1, NULL, FALSE);
                if (temp_border) bottom_border = temp_border;
                char temp_lpstr[30];
                std::string temp_str;
                GetDlgItemTextA(hDlg, IDC_EDIT5, temp_lpstr, 30);
                temp_str = temp_lpstr;
                if (temp_str.length() > 0) load = temp_str;
                GetDlgItemTextA(hDlg, IDC_EDIT6, temp_lpstr, 30);
                temp_str = temp_lpstr;
                if (temp_str.length() > 0) save = temp_str;
                auto temp_iter = GetDlgItemInt(hDlg, IDC_EDIT4, NULL, FALSE);
                if (temp_iter) iterations = temp_iter;
                HMENU hMenu = GetSubMenu(GetMenu(GetParent(hDlg)), 2);
                EnableMenuItem(hMenu, ID_SET_ROB, MF_ENABLED);
                EnableMenuItem(hMenu, ID_SET_OBS, MF_ENABLED);
                EnableMenuItem(hMenu, ID_SET_GOAL, MF_ENABLED);
                show_path = 0;
                SelectClipRgn(GetDC(GetParent(hDlg)), CreateRectRgn(0, 0, right_border + 1, bottom_border + 1));
                DrawMenuBar(GetParent(hDlg));
                InvalidateRect(GetParent(hDlg), NULL, TRUE);
            }

            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
