/*11327117 黃郁婷 11327143 廖宇晨*/
#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>      // setw
#include <sstream>      // istringstream
#include <cmath>        // log

struct Data {
    int serial;
    std::string school_code;
    std::string school_name;
    std::string dept_code;
    std::string dept_name;
    std::string day_type;      // 日間/進修別（內含空格）
    std::string level;         // 等級別（內含空格）
    int students;
    int aboriginal;
    int graduates;             // 上學年度畢業生 ← heap key
    std::string city;          // 縣市名稱（內含空格）
    std::string system;        // 體系別（內含空格）
};

struct Record {
    int serial;
    int graduate;
};

struct Node {
    bool isLeaf;
    std::vector<int> keys; // 存放畢業生數（唯一值，由小到大）
    std::vector<std::vector<int>> idLists; // 對應每個 key 的序號列表
    std::vector<Node*> children;
    Node* parent;

    Node(Node* p, bool leaf) {
        parent = p;
        isLeaf = leaf; 
    }

};

class TwoThreeTree {
  private:
    Node *root;
    
    void split(Node* node) { // 遞迴，維護23樹的性質
        // 準備分裂成 node1 (左), node2 (右)
        Node* node1 = new Node(node->parent, node->isLeaf);
        Node* node2 = new Node(node->parent, node->isLeaf);

        // 最小 key 給 node1, 最大 key 給 node2
        node1->keys.push_back(node->keys[0]);
        node1->idLists.push_back(node->idLists[0]);
        node2->keys.push_back(node->keys[2]);
        node2->idLists.push_back(node->idLists[2]);

        // 如果不是葉子，重新分配子節點
        if (!node->isLeaf) {
            node1->children = {node->children[0], node->children[1]};
            node2->children = {node->children[2], node->children[3]};
            for (Node* child : node1->children) child->parent = node1;
            for (Node* child : node2->children) child->parent = node2;
        }

        int midKey = node->keys[1];
        std::vector<int> midIdList = node->idLists[1];

        if (node == root) {
            // 建立新根節點
            Node* newRoot = new Node(nullptr, false);
            newRoot->keys.push_back(midKey);
            newRoot->idLists.push_back(midIdList);
            newRoot->children = {node1, node2};
            node1->parent = node2->parent = newRoot;
            root = newRoot;
        } else {
            Node* p = node->parent;
            // 將 midKey 插入父節點 p
            auto it = lower_bound(p->keys.begin(), p->keys.end(), midKey);
            int pos = distance(p->keys.begin(), it);
            p->keys.insert(it, midKey);
            p->idLists.insert(p->idLists.begin() + pos, midIdList);

            // 更新父節點的子節點指針
            p->children.erase(p->children.begin() + pos); // 移除原本指向 node 的指標
            p->children.insert(p->children.begin() + pos, {node1, node2});

            // 遞迴檢查父節點是否溢位
            if (p->keys.size() == 3) split(p);
        }
        delete node; // 釋放舊節點
    }

    // 取得樹高：2-3 樹所有葉子都在同一層，只需向左下找
    int GetHeight(Node* node) {
        if (node == nullptr) return 0;
        // 如果是葉子，高度為 1
        if (node->isLeaf) return 1;
        // 否則，高度為 1 加上子樹的高度
        return 1 + GetHeight(node->children[0]);
    }

    // 取得總節點數：遞迴走訪每個節點
    int CountNodes(Node* node) {
        if (node == nullptr) return 0;
        int count = 1; // 計算當前這個節點
        if (!node->isLeaf) {
            // 如果不是葉子，累加所有小孩的節點數
            for (Node* child : node->children) {
                count += CountNodes(child);
            }
        }
        return count;
    }

  public:
    TwoThreeTree() {
        root = nullptr;
    }

    void insertItem(int gradCount, int id) {
        // 如果是第一筆資料直接插入
        if (root == nullptr) {
            root = new Node(nullptr, true);
            root->keys.push_back(gradCount);
            root->idLists.push_back({id});
            return;
        }

        /*
        1. 尋找目標節點 (向下搜尋)，使用迴圈
            結束條件為當前節點是葉子(leaf)，如果不滿足條件就繼續往下
            如果當前節點(curr)只有1個key(也就是curr->key.size() == 1)，檢查curr->key與插入資料(grandCount)的大小關係：
                (1)如果curr->key[0] < grandCount，往下找最右邊的小孩(curr->children[size() - 1])
                (2)如果curr->key[0] > grandCount，往下找最左邊的小孩(curr->children[0])
                (3)如果curr->key[0] = grandCount，將插入資料的序號加入當前節點的idList，直接return
            如果當前節點(curr)有2個key(也就是curr->key.size() == 2)，檢查curr->key與插入資料(grandCount)的大小關係：
                (1)如果curr->key[1] < grandCount，往下找最右邊的小孩(curr->children[size() - 1])
                (2)如果curr->key[0] > grandCount，往下找最左邊的小孩(curr->children[0])
                (3)如果curr->key[1] > grandCount > curr->key[0]，往下找中間的小孩(curr->children[1])
                (4)如果curr->key[0] = grandCount，將插入資料的序號加入當前節點的idList，直接return
        */ 
        Node* curr = root;
        while (!curr->isLeaf) {
            // 先處理當前節點可能出現重複資料的情況
            for (int i = 0; i < curr->keys.size(); ++i) {
                if (gradCount == curr->keys[i]) {
                    curr->idLists[i].push_back(id);
                    return;
                }
            }

            // 依照節點內的 key 數量決定去哪個小孩
            if (curr->keys.size() == 1) {
                // (1) 如果 curr->key[0] < gradCount，往下找最右邊的小孩
                if (gradCount > curr->keys[0]) {
                    curr = curr->children[1]; // size 為 1 時，children[1] 是最右邊
                } 
                // (2) 如果 curr->key[0] > gradCount，往下找最左邊的小孩
                else {
                    curr = curr->children[0];
                }
            } 
            else if (curr->keys.size() == 2) {
                // (1) 如果 curr->key[1] < gradCount，往下找最右邊的小孩
                if (gradCount > curr->keys[1]) {
                    curr = curr->children[2]; // size 為 2 時，children[2] 是最右邊
                }
                // (2) 如果 curr->key[0] > gradCount，往下找最左邊的小孩
                else if (gradCount < curr->keys[0]) {
                    curr = curr->children[0];
                }
                // (3) 如果 curr->key[1] > gradCount > curr->key[0]，往下找中間的小孩
                else {
                    curr = curr->children[1];
                }
            }
        }

        /*
        2. 經由第一步找到leaf之後，要在這個節點依照key的大小排序
            首先檢查當前節點所有的key有沒有其中一個與插入資料(gradCount)相同
            如果有，加到該key的idList裡面，直接return
        */ 
        for (int i = 0; i < curr->keys.size(); ++i) {
            if (curr->keys[i] == gradCount) {
                curr->idLists[i].push_back(id);
                return;
            }
        }

        
        /*
        3. 將 gradCount 設為一個新的 key 並將所有的 key 由小至大進行排序
           使用 lower_bound 尋找應插入的位置，以維持 keys 的有序性
        */
   
        // 1. 先把資料放進去
        curr->keys.push_back(gradCount);
        curr->idLists.push_back({id});

        // 2. 手動跑一次類似「插入排序」的邏輯（由後往前比）
        for (int i = curr->keys.size() - 1; i > 0; i--) {
            if (curr->keys[i] < curr->keys[i-1]) {
                // 同步交換數值
                std::swap(curr->keys[i], curr->keys[i-1]);
                // 同步交換序號清單
                std::swap(curr->idLists[i], curr->idLists[i-1]);
            } else {
                break; // 已經定位好了
            }
        }
        /*
        4. 若節點溢位（有 3 個 keys），執行分裂
           分裂會將中間值上提，並建立兩個新的節點取代舊節點
        */
        if (curr->keys.size() == 3) {
            split(curr);
        }
    }

    void ShowRootData(std::vector<Data> &datalist) {
        int count = 1;
        int height = GetHeight(root);
        std::cout << "Tree height = " << GetHeight(root) << "\n";
        std::cout << "Number of nodes = " << CountNodes(root) << "\n";
        for (int i = 0; i < root->idLists.size(); i++) {
            for (int j = 0; j < root->idLists[i].size(); j++) {
                std::cout << count << ": [" << datalist[root->idLists[i][j] - 1].serial << "] ";
                std::cout << datalist[root->idLists[i][j] - 1].school_name << ", ";
                std::cout << datalist[root->idLists[i][j] - 1].dept_name << ", ";
                std::cout << datalist[root->idLists[i][j] - 1].day_type << ", ";
                std::cout << datalist[root->idLists[i][j] - 1].level << ", ";
                std::cout << datalist[root->idLists[i][j] - 1].students << ", ";
                std::cout << datalist[root->idLists[i][j] - 1].graduates << "\n";
                count++;
            }
        }
        std::cout << "\n" << std::endl;
    }
};

int ParseNumber(std::string temp) {
    std::string num = "";
    for (int i = 0; i < temp.size(); i++) {
        if (temp[i] != '"' && temp[i] != ',') {
            num = num + temp[i];
        }
    }
    return std::stoi(num);
}

bool ReadFile(std::vector<Data> &datalist) {
    std::string name;
    std::string filename;
    while (true) {
        std::cout << "\nInput a file number ([0] Quit): ";
        std::cin >> name;
        if (name == "0") {
            return false;
        }
        filename = "input" + name + ".txt";
        std::ifstream inputFile(filename);
        if (!inputFile.is_open()) {
            std::cout << "\n### " << filename << " does not exist! ###" << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        } else {
            break;
        }
        
    }
    std::ifstream inputFile(filename);
    std::string line;
    std::string temp;
    int i = 1;
    std::getline(inputFile, line);
    std::getline(inputFile, line);
    std::getline(inputFile, line);      // 前三行
    while (std::getline(inputFile, line)) {
        Data newdata;
        newdata.serial = i;
        i++;
        std::istringstream ss(line);
        std::getline(ss, newdata.school_code, '\t');
        std::getline(ss, newdata.school_name, '\t');
        std::getline(ss, newdata.dept_code, '\t');
        std::getline(ss, newdata.dept_name, '\t');
        std::getline(ss, newdata.day_type, '\t');
        std::getline(ss, newdata.level, '\t');
        std::getline(ss, temp, '\t');
        newdata.students = ParseNumber(temp);
        std::getline(ss, temp, '\t');
        newdata.aboriginal = ParseNumber(temp);
        std::getline(ss, temp, '\t');
        newdata.graduates = ParseNumber(temp);
        std::getline(ss, newdata.city, '\t');
        std::getline(ss, newdata.system);
        datalist.push_back(newdata);
    }
    return true;
}

void Mission1(std::vector<Data> &datalist) {
    if (!ReadFile(datalist)) {
        std::cout << "\n";
        return;
    }
    TwoThreeTree Tree23;
    for (int i = 0; i < datalist.size(); i++) {
        Tree23.insertItem(datalist[i].graduates, datalist[i].serial);
    }
    Tree23.ShowRootData(datalist);

}

void Mission2(std::vector<Data> &datalist) {
    if (datalist.size() == 0) {
        std::cout << "\n### Choose 1 first. ###\n" << std::endl;
        return;
    }
}

void Mission3() {
    
}

void Mission4() {
    
}

void OutputFile() {
    std::string filename;
    std::ofstream outputfile(filename);
    outputfile.close();
}
void LetsGo() {
    std::cout << "* Data Structures and Algorithms *" << std::endl;
    std::cout << "****** Balanced Search Tree ******" << std::endl;
    std::cout << "* 0. QUIT                        *" << std::endl;
    std::cout << "* 1. Build 23 tree               *" << std::endl;
    std::cout << "* 2. Build AVL tree              *" << std::endl;
    std::cout << "**********************************" << std::endl;
    std::cout << "Input a choice(0, 1, 2): ";
    int command;
    std::vector<Data> datalist;
    while (std::cin >> command) {
        if (command == 0) {
            break;
        } else if (command == 1) {
            if (datalist.size() != 0) {
                datalist.clear();
            }
            Mission1(datalist);
        } else if (command == 2) {
            Mission2(datalist);
        } else {
            std::cout << "\nCommand does not exist!\n" << std::endl;
        }
        std::cout << "* Data Structures and Algorithms *" << std::endl;
        std::cout << "****** Balanced Search Tree ******" << std::endl;
        std::cout << "* 0. QUIT                        *" << std::endl;
        std::cout << "* 1. Build 23 tree               *" << std::endl;
        std::cout << "* 2. Build AVL tree              *" << std::endl;
        std::cout << "**********************************" << std::endl;
        std::cout << "Input a choice(0, 1, 2): ";
    }
}

int main() {
    LetsGo();
    return 0;
}
