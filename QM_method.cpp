#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <cmath>

using namespace std;

/**
 * 此struct有兩個元素：
 * 1. vector<int> minterms：用來儲存此term是由哪些項合併而來的
 * 2. vector<int> value：此term對應的二進制0/1/-1數列，-1代表合併項(-)
**/
struct GroupElement
{
    vector<int> minterms;
    vector<int> value;
};

pair<vector<int>, int> getMintermAndCount(const int minTerm, const int literalCount);
map<int, vector<GroupElement>> mergeGroupElement(map<int, vector<GroupElement>> numberedGroups, int literals, vector<GroupElement> &notUpdated);
vector<pair<vector<int>, vector<int>>> getprimeImplicantGroup(map<int, vector<GroupElement>> numberedGroups, vector<GroupElement> notUpdated, set<int> minTermsSet);
set<vector<int>> getEssentialSet(vector<pair<vector<int>, vector<int>>> primeImplicantGroup, set<int> minTermsSet);
void shrinkByProcessedItems(vector<pair<vector<int>, vector<int>>> &primeImplicantGroup, set<int> &minTermsSet, set<vector<int>> essential);
set<vector<int>> getAnswerSet(vector<pair<vector<int>, vector<int>>> primeImplicantGroup, set<int> minTermsSet, set<vector<int>> essential);
void displayFormatedAnswer(set<vector<int>> answer);
void displayFormatedEssential(set<vector<int>> essential);
void printMinterm(const vector<int> v);
void printState(const vector<int> v);
void printMintermWithDontCare(const vector<int> v, const set<int> dontCareSet);
void printGroups(const map<int, vector<GroupElement>> allGroups, const set<int> dontCareSet);
void printNotUpdated(vector<GroupElement> notUpdated);
void printPrimeImplicantsTable(vector<pair<vector<int>, vector<int>>> primeImplicantGroup, set<int> minTermsSet, int literals);

int main()
{
    cout << "\nQuine McCluskey Method\n";
    cout << "\n--------------------------------------------------\n";   // 資料輸入讀取部分
    char choice = 'y';
    stringstream ss("");
    string line;
    while (choice == 'Y' || choice == 'y')
    {
        int literalCount;
        cout << "Enter the number of Literals(greather than Zero): ";   // 輸入變數數目
        cin.clear();    // 清空cin buffer
        cin.sync();
        cin >> literalCount;

        set<int> minTermsSet;
        cout << "Enter Minterms with space betweeen(eg. 1 2 3): " << endl;  // 輸入最小項
        ss.clear(); // 清空ss
        ss.str("");
        cin.clear();
        cin.sync();
        getline(cin, line);
        ss << line;
        int minterm;
        while (ss >> minterm)
            minTermsSet.insert(minterm);

        set<int> dontCareTermsSet;
        string check;
        cout << "Does the function have don't care(s)? Y/N: ";  // 詢問是否有不理會項
        cin.clear();
        cin.sync();
        cin >> check;
        if (check == "Y" || check == "y")   // 輸入不理會項
        {
            cout << "Enter Dont Care Terms with space betweeen(eg. 1 2 3): " << endl;   
            ss.clear();
            ss.str("");
            cin.clear();
            cin.sync();
            getline(cin, line);
            ss << line;
            int dontCareTerm;
            while (ss >> dontCareTerm)
                dontCareTermsSet.insert(dontCareTerm);
        }

        cout << "\n--------------------------------------------------\n";   // 資料轉二進制進行初步處理與分類
        map<int, vector<GroupElement>> numberedGroups; // 以1的數量作為key進行分組的map
        set<int> totalTerms;    // totalTerms為minTermsSet + dontCareTermsSet
        totalTerms.insert(minTermsSet.begin(), minTermsSet.end());
        totalTerms.insert(dontCareTermsSet.begin(), dontCareTermsSet.end());
        for (auto term : totalTerms)
        {
            pair<vector<int>, int> result = getMintermAndCount(term, literalCount); 
            GroupElement groupEle;
            // 構造groupEle
            groupEle.minterms.push_back(term);  
            groupEle.value = result.first;
            // 將groupEle存入相應的組別
            numberedGroups[result.second].push_back(groupEle);
            // 印出 十進位最小項：二進位最小項
            cout << setw(2) << left << term << ": ";
            printMinterm(result.first);
            cout << endl;
        }

        cout << "\n--------------------------------------------------\n";   // 合併與印出以1的數量進行分組的表格
        cout << "Grouped Minterms And DontCares: \n\n"; // 打印以1的數量進行分組的表格(尚未進行任何合併)
        printGroups(numberedGroups, dontCareTermsSet);  
        vector<GroupElement> notUpdated;    // 用來儲存未被合併到遺留下來的項
        
        // 合併同時進行打印表格
        while (true)
        {
            map<int, vector<GroupElement>> newNumberedGroups = mergeGroupElement(numberedGroups, literalCount, notUpdated);
            if (!newNumberedGroups.size())  // 當newNumberedGroups為一空map時，跳出迴圈(已無法再進行合併)
                break;
            numberedGroups = newNumberedGroups;
            cout << "\n--------------------------------------------------\n";
            cout << "Grouped Minterms And DontCares: \n\n";
            printGroups(numberedGroups, dontCareTermsSet); // 打印以1的數量進行分組的表格(更新後)
        }
	    cout << "\n--------------------------------------------------\n";   // 打印未被合併到遺留下來的項
	    cout << "Terms which were left when merging the groups" << endl;
	    printNotUpdated(notUpdated);

        cout << "\n--------------------------------------------------\n";   // 建構及打印prime implicant table
        vector<pair<vector<int>, vector<int>>> primeImplicantGroup;
        primeImplicantGroup = getprimeImplicantGroup(numberedGroups, notUpdated, minTermsSet);
        cout << "Prime implicants table\n\n";
        printPrimeImplicantsTable(primeImplicantGroup, minTermsSet, literalCount);

        set<vector<int>> essential; 
        essential = getEssentialSet(primeImplicantGroup, minTermsSet);  // 找出所有基本質含項

        cout << "--------------------------------------------------\n"; // 縮減primeImplicantGroup並輸出縮減後的table
        shrinkByProcessedItems(primeImplicantGroup, minTermsSet, essential);
        cout << "Shinked prime implicants table\n";
        cout << "If the table is empty, it means that all prime implicants are essential prime implicants\n\n";
        printPrimeImplicantsTable(primeImplicantGroup, minTermsSet, literalCount);
        
        cout << "--------------------------------------------------\n"; // 從表格中找pivot以找出及輸出所有基本質含項
        cout << "Essential prime implicants:\n";
        displayFormatedEssential(essential);
        cout << endl;

        cout << "--------------------------------------------------\n"; // 利用窮舉法找出最簡解並格式化輸出answer(SOP)
        set<vector<int>> answer;
        answer = getAnswerSet(primeImplicantGroup, minTermsSet, essential);
        cout << "The Minimized Equation is:\n";
        displayFormatedAnswer(answer);
        cout << endl;

        cout << "--------------------------------------------------\n"; // 詢問是否再次執行此程式
        cout << "Try again?(Y/N): ";
        cin.clear();
        cin.sync();
        cin >> choice;
    }

    system("pause");
    return 0;
}

/**
 * param1：最小項
 * param2：變數數量
 * return：pair<vector<int>, int>
 * first：一個由minTerm轉換的二進制0/1數列(vector<int>)
 * second：二進制0/1數列中1的個數
**/
pair<vector<int>, int> getMintermAndCount(const int minTerm, const int literalCount)
{
    vector<int> mintermVector;
    int onesCount = 0;

    for (int i = 0; i < literalCount; ++i)
    {
        if (minTerm & 1 << (literalCount - i - 1))
        {
            mintermVector.push_back(1);
            onesCount++;
        }
        else
        {
            mintermVector.push_back(0);
        }
    }

    pair<vector<int>, int> mintermAndCount;
    mintermAndCount.first = mintermVector;
    mintermAndCount.second = onesCount;

    return mintermAndCount;
}

/**
 * param1：以1的數量作為key的map(舊版)
 * param2：變數數量
 * param3：用來儲存未被合併到遺留下來的項之vector
 * return：以1的數量作為key的map(新版)
 * 若沒有進行任何合併，將回傳一個空map{}
 * 將僅一位元不同的項進行合併，並回傳新版的map
**/
map<int, vector<GroupElement>> mergeGroupElement(map<int, vector<GroupElement>> numberedGroups, int literals, vector<GroupElement> &notUpdated)
{
    map<int, vector<GroupElement>> newNumberedGroups;   // 以1的數量作為key的map(新版)
    int updateCount = 0;    // 記錄合併了幾組資料
    set<vector<int>> termsUsed; // 儲存有哪些最小項已被合併過

    for (auto &group : numberedGroups)
    {
        if (numberedGroups.count(group.first + 1))  // 如果下一個組別存在，才進行位元比較
        {
            for (auto &groupElement : group.second)
            {
                for (auto &nextGroupElement : numberedGroups[group.first + 1])
                {
                    int diffIndex = -1; // 儲存第幾位元的值不同(index)
                    int diffCount = 0;  // 儲存有幾個位元不同
                    for (int i = 0; i < literals; ++i)
                    {
                        if (groupElement.value[i] != nextGroupElement.value[i])
                        {
                            diffIndex = i;
                            diffCount++;
                        }
                    }

                    if (diffCount == 1) // 僅一個位元不同才符合合併條件
                    {
                        updateCount++;  
                        GroupElement newGroupElement;
                        // 構造newGroupElement的value部分
                        newGroupElement.value = groupElement.value;
                        newGroupElement.value[diffIndex] = -1;  // 相異位元記為-1(已合併)
                        // 構造newGroupElement的minterm部分
                        newGroupElement.minterms = groupElement.minterms;
                        for (auto &m : nextGroupElement.minterms)
                            newGroupElement.minterms.push_back(m);
                        // 將已合併過的最小項記錄下來
                        termsUsed.insert(groupElement.minterms);
                        termsUsed.insert(nextGroupElement.minterms);
                        // 將newGroupElement存入map的相應組別
                        newNumberedGroups[group.first].push_back(newGroupElement);
                    }
                }
            }
        }
    }

    // 如果沒有任何一組資料可以進行合併了，回傳一個空的map做代表
    if (updateCount == 0)   
        return {};

    // 將舊map合併的資料拷貝到新map
    for (auto &group : numberedGroups)
    {
        for (auto &groupElement : group.second)
        {
            if (!termsUsed.count(groupElement.minterms))
                notUpdated.push_back(groupElement);
        }
    }

    return newNumberedGroups;
}

/**
 * param1：以1的數量作為key的map(最終版)
 * param2：集合所有遺留下來的項之vector
 * param3：最小項集合而成的set
 * return：一個可用來建prime implicant table的vector<pair<vector<int>, vector<int>>>
 * first of the pair：用來存此prime implicant轉換而來的二進制0/1數列
 * second of the pair：用來存此prime implicant可以照顧到哪些最小項
**/
vector<pair<vector<int>, vector<int>>> getprimeImplicantGroup(map<int, vector<GroupElement>> numberedGroups, vector<GroupElement> notUpdated, set<int> minTermsSet)
{
    vector<pair<vector<int>, vector<int>>> primeImplicantGroup;
    // notUpdated去重
    for (int i = 0; i < notUpdated.size(); i++)
    {
        for (int j = i + 1; j < notUpdated.size(); j++)
        {
            if (notUpdated[i].value == notUpdated[j].value)
            {
                notUpdated.erase(notUpdated.begin() + j);
                notUpdated.shrink_to_fit();
                j--;
            }
        }
    }
    // 被遺留下來的項必為prime implicant，所以直接存入primeImplicantGroup
    for (auto &Ele : notUpdated)    
    {
        pair<vector<int>, vector<int>> tempPair;
        tempPair.first = Ele.value;
        for (auto &minTerm : Ele.minterms)
        {
            if (minTermsSet.count(minTerm))
                tempPair.second.push_back(minTerm);
        }
        primeImplicantGroup.push_back(tempPair);
    }
    // 對numberedGroups進行去重
    for (auto &groupPair : numberedGroups)
    {
        for (int i = 0; i < groupPair.second.size(); i++)
        {
            for (int j = i + 1; j < groupPair.second.size(); j++)
            {
                if (groupPair.second[i].value == groupPair.second[j].value)
                {
                    groupPair.second.erase(groupPair.second.begin() + j);
                    groupPair.second.shrink_to_fit();
                    j--;
                }
            }
        }
    }
    // 把已經去重過的numberedGroups也存到primeImplicantGroup
    for (auto &groupPair : numberedGroups)
    {
        for (auto &Ele : groupPair.second)
        {
            pair<vector<int>, vector<int>> tempPair;
            tempPair.first = Ele.value;
            for (auto &minTerm : Ele.minterms)
            {
                if (minTermsSet.count(minTerm))
                    tempPair.second.push_back(minTerm);
            }
            primeImplicantGroup.push_back(tempPair);
        }
    }

    return primeImplicantGroup;
}

/**
 * param1：primeImplicantGroup
 * param2：最小項集合而成的set
 * return：一個由基本質含項所成的set<vector<int>>
 * 找出基本質含項，包裝成一個set並回傳
**/
set<vector<int>> getEssentialSet(vector<pair<vector<int>, vector<int>>> primeImplicantGroup, set<int> minTermsSet)
{
    map<int, int> mintermMap;   // 以minterm之值作為key，儲存各項minterm被質含項照顧到幾次
    for (auto &term : primeImplicantGroup)  // 建構mintermMap
    {
        for (auto &minterm : term.second)
        {
            mintermMap[minterm]++;
        }
    }

    set<vector<int>> essential;
    for (auto &minterm : mintermMap)    // 找出基本質含項並儲存至essential
    {
        if (minterm.second == 1)
        {
            for (auto &term : primeImplicantGroup)
            {
                if (count(term.second.begin(), term.second.end(), minterm.first))
                {
                    essential.insert(term.first);
                    break;
                }
            }
        }
    }

    return essential;
}

/**
 * param1：primeImplicantGroup
 * param2：最小項集合而成的set
 * param3：基本質含項所組成的set
 * 此函數可利用essential去除已被照顧到的最小項(調整minTermsSet)
 * 並以此縮減去除primeImplicantGroup不必要的東西
**/
void shrinkByProcessedItems(vector<pair<vector<int>, vector<int>>> &primeImplicantGroup, set<int> &minTermsSet, set<vector<int>> essential)
{   
    pair<vector<int>, vector<int>> tempPair;
    for (auto &term : essential)
    {
        // 利用0/1/-1數列，將相應的pair複製一份出來
        for (auto &Ele : primeImplicantGroup)   
        {
            if (Ele.first == term)
            {
                tempPair = Ele;
                break;
            }
        }
        // primeImplicantGroup去除基本質含項
        primeImplicantGroup.erase(remove(primeImplicantGroup.begin(), primeImplicantGroup.end(), tempPair), primeImplicantGroup.end());
        primeImplicantGroup.shrink_to_fit();
        // 去除已被照顧到的最小項
        for (auto &Ele : primeImplicantGroup)   
        {
            for (auto &minterm : tempPair.second)
            {
                Ele.second.erase(remove(Ele.second.begin(), Ele.second.end(), minterm), Ele.second.end());
                Ele.second.shrink_to_fit();
            }
        }
        // minTermsSet去除已被照顧到的最小項
        for (auto &minterm : tempPair.second)
            minTermsSet.erase(minterm);
    }
    // primeImplicantGroup去除已無法照顧到任何最小項的元素
    for (auto &Ele : primeImplicantGroup)
    {
        if (!Ele.second.size())
        {
            primeImplicantGroup.erase(remove(primeImplicantGroup.begin(), primeImplicantGroup.end(), Ele), primeImplicantGroup.end());
            primeImplicantGroup.shrink_to_fit();
        }
    }
}

/**
 * param1：以1的數量作為key的map(縮減版)
 * param2：縮減後的minTermsSet
 * param3：基本質含項所組成的set
 * return answer set，此set的組成元素為若干個二進制0/1/-1vector
 * 表示最終化簡後的解
**/
set<vector<int>> getAnswerSet(vector<pair<vector<int>, vector<int>>> primeImplicantGroup, set<int> minTermsSet, set<vector<int>> essential)
{   
    set<vector<int>> answer;
    int size = primeImplicantGroup.size();

    if (!size)    // 如果primeImplicantGroup為空， 代表化簡後的結果皆為基本質含項
    {
        answer = essential;
        return answer;
    }

    int permutationNumber = pow(2, size);
    vector<string> selectFlagString;    // 用二進制0/1字串來代表相應項要選還是不選
    for (int i = 0; i < permutationNumber; ++i) // 將10進位整數轉換為長度為size的二進制字串，並存入selectFlagString
    {
        string temp;
        int num = i;
        while (true)
        {
            temp += to_string(num % 2);
            num /= 2;
            if (!num)
                break;
        }
        while (temp.length() != size)   // 將字串長度補成(size)長
            temp += "0";
        reverse(temp.begin(), temp.end());
        selectFlagString.push_back(temp);
    }

    auto countOnesNum = [](string & str) -> int // 用來算1的個數的函數
        {
            int count = 0;
            for (auto &ch : str)
                if (ch == '1')
                    count++;
            return count;
        }; 

    unsigned int min = 0xffffffff;  // 用來儲存額外還需幾項才可把全部的最小項照顧到
    string finalFlag;   // 用來儲存最終的flag字串
    for (auto &flag : selectFlagString)
    {   
        set<int> tempMinTermsSet = minTermsSet;
        for (int i = 0; i < size; ++i)
        {
            if (flag[i] == '1') // '1'代表此對應項要拿來照顧剩餘最小項
            {
                for (auto &minterm : primeImplicantGroup[i].second)
                    tempMinTermsSet.erase(minterm);
            }
        }
        if (!tempMinTermsSet.size())    // 如果所有剩餘最小項都有被照顧到
        {   
            int onesCount = countOnesNum(flag); // 用來儲存當前候選flag中1的個數
            if (onesCount < min)    // 更新min和finalFlag
            {   
                min = onesCount;
                finalFlag = flag;
            }
        }
    }
    
    answer = essential;
    for (int i = 0; i < finalFlag.size(); ++i)  // 根據finalFlag抓取相應的term存入vector
        if (finalFlag[i] == '1')
            answer.insert(primeImplicantGroup[i].first); 

    return answer;
}

/**
 * param1：set<vector<int>> answer
 * 將set<vector<int>> answer進行格式化處理並以SOP形式輸出
**/
void displayFormatedAnswer(set<vector<int>> answer)
{
    for (auto &i : answer)
    {
        char letter = 'a';
        for (auto &j : i)
        {
            if (j != -1)
            {
                if (j == 1)
                    cout << letter;
                else
                    cout << letter << "'";
            }
            letter++;
        }
        cout << " + ";
    }
    cout << "\b\b" << "  " << endl; // 把多餘的+去掉
}

/**
 * param1：set<vector<int>> essential
 * 將set<vector<int>> essential進行格式化處理並輸出
**/
void displayFormatedEssential(set<vector<int>> essential)
{
    for (auto &i : essential)
    {
        char letter = 'a';
        for (auto &j : i)
        {
            if (j != -1)
            {
                if (j == 1)
                    cout << letter;
                else
                    cout << letter << "'";
            }
            letter++;
        }
        cout << ", ";
    }
    cout << "\b\b" << "  " << endl; 
}

/**
 * param1：二進制0/1數列(vector<int>)
 * 將最小項0/1vector印出
**/
void printMinterm(const vector<int> v)
{
    for (int i = 0; i < v.size(); ++i)
        cout << setw(3) << left << v[i];
}

/**
 * param1：二進制0/1數列(vector<int>)
 * 將最小項0/1vector印出，若有合併項(-1)，印出-代替
**/
void printState(const vector<int> v)
{
    for (int i = 0; i < v.size(); ++i)
    {
        if (v[i] == -1)
            cout << "-";
        else
            cout << v[i];
    }
}

/**
 * param1：十進位制最小項組成的vector
 * param2：十進位制不理會項的set(用以判斷param1有哪些最小項是不理會項)
 * 將十進位制最小項組成的vector以空格隔開，不理會項加上*作為標誌
**/
void printMintermWithDontCare(const vector<int> v, const set<int> dontCareSet)
{
    for (int i = 0; i < v.size(); ++i)
        cout << setw(2) << v[i] << ((dontCareSet.count(v[i])) ? "* " : "  ");
}

/**
 * param1：以1的數量作為key的map
 * param2：十進位制不理會項的set
 * 打印出以1的數量進行分組的表格
**/
void printGroups(const map<int, vector<GroupElement>> allGroups, const set<int> dontCareSet)
{
    for (auto &group : allGroups)
    {
        cout << "Group " << group.first << endl;
        for (auto &groupElement : group.second)
        {
            printMintermWithDontCare(groupElement.minterms, dontCareSet);
            cout << setw(2) << left << ": ";
            printState(groupElement.value);
            cout << endl;
        }
    }
}

/**
 * param1：集合所有遺留下來的項之vector
 * 將vector<GroupElement> notUpdated格式化印出
**/
void printNotUpdated(vector<GroupElement> notUpdated)
{
    cout << endl;
    for (auto &groupElement : notUpdated)
    {
        printMinterm(groupElement.minterms);
        cout << ": ";
        printState(groupElement.value);
        cout << endl;
    }
    cout << endl;
}

/**
 * param1：primeImplicantGroup
 * param2：最小項集合而成的set
 * param3：變數數量(算寬度用)
 * 印出初始的prime implicant table
**/
void printPrimeImplicantsTable(vector<pair<vector<int>, vector<int>>> primeImplicantGroup, set<int> minTermsSet, int literals)
{
    cout << setw(literals + 2) << left << "terms";
    cout << " | ";
    for (auto &minterm : minTermsSet)
        cout << setw(4) << left << minterm;
    cout << endl;
    for (auto & term : primeImplicantGroup)
    {   
        if (term.second.size())     // 可以照顧到剩餘最小項的term再打印
        {
            printState(term.first);
            for (int i = 0; i < (literals + 2) - term.first.size(); i++)
                cout << " ";
            cout << " | ";
            for (auto &minterm : minTermsSet)
            {
                if (count(term.second.begin(), term.second.end(), minterm))
                    cout << setw(4) << left << "X";
                else 
                    cout << "    ";
            }
            cout << endl;
        }
    }
    cout << "\n\n";
}
