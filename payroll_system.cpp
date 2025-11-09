#include <windows.h>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>
#include <algorithm>

#define ID_NAME 1
#define ID_HOURS 2
#define ID_RATE 3
#define ID_DEDUCTIONS 4
#define ID_BENEFITS 5
#define ID_CALCULATE 6
#define ID_SAVE 7
#define ID_CLEAR 8
#define ID_SEARCH 9
#define ID_SORT 10
#define ID_DELETE 11
#define ID_EXIT 12
#define ID_RESULT 13
#define ID_LISTBOX 14

struct Employee {
    std::string name;
    double hours, rate, deductions, benefits, salary;
};

std::vector<Employee> employees;

double CalculateSalary(double hours, double rate, double deductions, double benefits) {
    const double overtimeThreshold = 40.0;
    const double overtimeMultiplier = 1.5;
    double base = (hours <= overtimeThreshold)
        ? hours * rate
        : overtimeThreshold * rate + (hours - overtimeThreshold) * rate * overtimeMultiplier;
    return base + benefits - deductions;
}

bool GetValidatedDouble(const std::string& str, double& value) {
    try { value = std::stod(str); return value >= 0.0; }
    catch (...) { return false; }
}

// format heading + table-style save
void SaveAllRecords() {
    std::ofstream fout("payroll_records.txt");
    fout << std::left << std::setw(15) << "Name"
         << std::setw(10) << "Hours"
         << std::setw(10) << "Rate"
         << std::setw(12) << "Deductions"
         << std::setw(10) << "Benefits"
         << std::setw(15) << "Total Salary"
         << "\n-------------------------------------------------------------\n";
    for (auto& e : employees) {
        fout << std::left << std::setw(15) << e.name
             << std::setw(10) << e.hours
             << std::setw(10) << e.rate
             << std::setw(12) << e.deductions
             << std::setw(10) << e.benefits
             << "Rs." << std::fixed << std::setprecision(2) << e.salary << "\n";
    }
}

void LoadRecords() {
    std::ifstream fin("payroll_records.txt");
    if (!fin.is_open()) return;
    std::string line;
    bool skipHeader = true;
    while (std::getline(fin, line)) {
        if (line.find('-') != std::string::npos || skipHeader) {
            skipHeader = false;
            continue;
        }
        if (line.empty()) continue;
        std::stringstream ss(line);
        Employee e;
        ss >> e.name >> e.hours >> e.rate >> e.deductions >> e.benefits;
        std::string rupee;
        ss >> rupee >> e.salary;
        employees.push_back(e);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HWND hName, hHours, hRate, hDed, hBen, hResult, hList;
    switch (msg) {
    case WM_CREATE:
        LoadRecords();

        CreateWindow("STATIC", "Employee Name:", WS_VISIBLE | WS_CHILD, 20, 20, 120, 25, hwnd, NULL, NULL, NULL);
        hName = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER, 150, 20, 200, 25, hwnd, (HMENU)ID_NAME, NULL, NULL);

        CreateWindow("STATIC", "Hours Worked:", WS_VISIBLE | WS_CHILD, 20, 60, 120, 25, hwnd, NULL, NULL, NULL);
        hHours = CreateWindow("EDIT", "40", WS_VISIBLE | WS_CHILD | WS_BORDER, 150, 60, 200, 25, hwnd, (HMENU)ID_HOURS, NULL, NULL);

        CreateWindow("STATIC", "Hourly Rate:", WS_VISIBLE | WS_CHILD, 20, 100, 120, 25, hwnd, NULL, NULL, NULL);
        hRate = CreateWindow("EDIT", "200", WS_VISIBLE | WS_CHILD | WS_BORDER, 150, 100, 200, 25, hwnd, (HMENU)ID_RATE, NULL, NULL);

        CreateWindow("STATIC", "Deductions:", WS_VISIBLE | WS_CHILD, 20, 140, 120, 25, hwnd, NULL, NULL, NULL);
        hDed = CreateWindow("EDIT", "0", WS_VISIBLE | WS_CHILD | WS_BORDER, 150, 140, 200, 25, hwnd, (HMENU)ID_DEDUCTIONS, NULL, NULL);

        CreateWindow("STATIC", "Benefits:", WS_VISIBLE | WS_CHILD, 20, 180, 120, 25, hwnd, NULL, NULL, NULL);
        hBen = CreateWindow("EDIT", "0", WS_VISIBLE | WS_CHILD | WS_BORDER, 150, 180, 200, 25, hwnd, (HMENU)ID_BENEFITS, NULL, NULL);

        CreateWindow("BUTTON", "Calculate", WS_VISIBLE | WS_CHILD, 20, 220, 100, 30, hwnd, (HMENU)ID_CALCULATE, NULL, NULL);
        CreateWindow("BUTTON", "Save", WS_VISIBLE | WS_CHILD, 130, 220, 100, 30, hwnd, (HMENU)ID_SAVE, NULL, NULL);
        CreateWindow("BUTTON", "Clear", WS_VISIBLE | WS_CHILD, 240, 220, 100, 30, hwnd, (HMENU)ID_CLEAR, NULL, NULL);

        CreateWindow("BUTTON", "Search", WS_VISIBLE | WS_CHILD, 20, 260, 100, 30, hwnd, (HMENU)ID_SEARCH, NULL, NULL);
        CreateWindow("BUTTON", "Sort", WS_VISIBLE | WS_CHILD, 130, 260, 100, 30, hwnd, (HMENU)ID_SORT, NULL, NULL);
        CreateWindow("BUTTON", "Delete", WS_VISIBLE | WS_CHILD, 240, 260, 100, 30, hwnd, (HMENU)ID_DELETE, NULL, NULL);
        CreateWindow("BUTTON", "Exit", WS_VISIBLE | WS_CHILD, 130, 300, 100, 30, hwnd, (HMENU)ID_EXIT, NULL, NULL);

        CreateWindow("STATIC", "Net Salary:", WS_VISIBLE | WS_CHILD, 20, 340, 120, 25, hwnd, NULL, NULL, NULL);
        hResult = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_READONLY, 150, 340, 200, 25, hwnd, (HMENU)ID_RESULT, NULL, NULL);

        hList = CreateWindow("LISTBOX", "", WS_VISIBLE | WS_CHILD | WS_BORDER | LBS_NOTIFY | WS_VSCROLL,
            370, 20, 330, 345, hwnd, (HMENU)ID_LISTBOX, NULL, NULL);

        for (auto& e : employees) {
            std::ostringstream ss;
            ss << e.name << " | Rs." << std::fixed << std::setprecision(2) << e.salary;
            SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)ss.str().c_str());
        }
        break;

    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case ID_CALCULATE: {
            char name[256], h[64], r[64], d[64], b[64];
            GetWindowText(hName, name, 256);
            GetWindowText(hHours, h, 64);
            GetWindowText(hRate, r, 64);
            GetWindowText(hDed, d, 64);
            GetWindowText(hBen, b, 64);
            if (strlen(name) == 0) { MessageBox(hwnd, "Enter name", "Error", MB_OK); break; }
            double hours, rate, ded, ben;
            if (!GetValidatedDouble(h, hours) || !GetValidatedDouble(r, rate) ||
                !GetValidatedDouble(d, ded) || !GetValidatedDouble(b, ben)) {
                MessageBox(hwnd, "Invalid input", "Error", MB_OK); break;
            }
            double sal = CalculateSalary(hours, rate, ded, ben);
            std::ostringstream ss; ss << "Rs." << std::fixed << std::setprecision(2) << sal;
            SetWindowText(hResult, ss.str().c_str());
            break;
        }
        case ID_SAVE: {
            char name[256], h[64], r[64], d[64], b[64], s[64];
            GetWindowText(hName, name, 256);
            GetWindowText(hHours, h, 64);
            GetWindowText(hRate, r, 64);
            GetWindowText(hDed, d, 64);
            GetWindowText(hBen, b, 64);
            GetWindowText(hResult, s, 64);
            if (strlen(name) == 0 || strlen(s) == 0) { MessageBox(hwnd, "Please calculate first", "Error", MB_OK); break; }
            Employee e{ name, std::stod(h), std::stod(r), std::stod(d), std::stod(b), std::stod(std::string(s + 3)) };
            employees.push_back(e);
            SaveAllRecords();
            std::ostringstream ss; ss << e.name << " | Rs." << std::fixed << std::setprecision(2) << e.salary;
            SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)ss.str().c_str());
            MessageBox(hwnd, "Record saved!", "Info", MB_OK);
            break;
        }
        case ID_CLEAR:
            SetWindowText(hName, ""); SetWindowText(hHours, "40");
            SetWindowText(hRate, "200"); SetWindowText(hDed, "0");
            SetWindowText(hBen, "0"); SetWindowText(hResult, ""); break;

        case ID_SEARCH: {
            char query[256]; GetWindowText(hName, query, 256);
            bool found = false;
            for (auto& e : employees)
                if (_stricmp(e.name.c_str(), query) == 0) {
                    std::ostringstream ss; ss << e.name << " — Rs." << e.salary;
                    MessageBox(hwnd, ss.str().c_str(), "Found", MB_OK); found = true; break;
                }
            if (!found) MessageBox(hwnd, "Employee not found", "Info", MB_OK);
            break;
        }

        case ID_SORT:
            std::sort(employees.begin(), employees.end(),
                [](const Employee& a, const Employee& b) { return a.name < b.name; });
            SendMessage(hList, LB_RESETCONTENT, 0, 0);
            for (auto& e : employees) {
                std::ostringstream ss;
                ss << e.name << " | Rs." << std::fixed << std::setprecision(2) << e.salary;
                SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)ss.str().c_str());
            }
            SaveAllRecords();
            break;

        case ID_DELETE: {
            int sel = SendMessage(hList, LB_GETCURSEL, 0, 0);
            if (sel != LB_ERR) {
                employees.erase(employees.begin() + sel);
                SendMessage(hList, LB_DELETESTRING, sel, 0);
                SaveAllRecords();
            }
            break;
        }

        case ID_EXIT: PostQuitMessage(0); break;
        }
        break;
    }
    case WM_DESTROY:
        SaveAllRecords();
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE h, HINSTANCE, LPSTR, int nCmdShow) {
    const char CLASS_NAME[] = "PayrollApp";
    WNDCLASS wc = { };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = h;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    HWND hwnd = CreateWindow(CLASS_NAME, "Payroll Management System",
        WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME, CW_USEDEFAULT, CW_USEDEFAULT, 740, 420,
        NULL, NULL, h, NULL);
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
