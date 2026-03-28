/*11327117 黃郁婷 11327143 廖宇晨*/
#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>      // setw
#include <sstream>      // istringstream
#include <cmath>        // log
#include <algorithm>    // sort

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

struct AVLNode {
    std::string schoolName;    // 鍵值 (Key)：學校名稱
    std::vector<int> idList;   // 存放該校所有系所的原始資料序號 (Serial)
    int height;                // 節點高度
    AVLNode *left, *right;

    AVLNode(std::string name, int id) 
        : schoolName(name), height(1), left(nullptr), right(nullptr) {
        idList.push_back(id);
    }
};

class TwoThreeTree {
  private:
    Node *root;

    int findInsertPos(const std::vector<int>& keys, int target) { // 使用指標傳遞，可以節省時間與記憶體
        int low = 0;
        int high = keys.size();
        while (low < high) { // binary search
            int mid = low + (high - low) / 2;
            if (keys[mid] < target) {
                low = mid + 1;
            } else {
                high = mid;
            }
        }
        return low;
    }

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
            int pos = findInsertPos(p->keys, midKey); // 找到中間值應該填入父節點的哪邊

            p->keys.insert(p->keys.begin() + pos, midKey);
            p->idLists.insert(p->idLists.begin() + pos, midIdList);

            // 更新父節點的子節點指針
            p->children.erase(p->children.begin() + pos); // 移除原本指向爆開的節點指標
            p->children.insert(p->children.begin() + pos, {node1, node2}); // 更新爆開節點的位置 

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
            bool found = false;
            for (int i = 0; i < curr->keys.size(); ++i) {
                if (gradCount == curr->keys[i]) { // 數值已存在，直接加入序號
                    curr->idLists[i].push_back(id);
                    return;
                }
                if (gradCount < curr->keys[i]) { // 每個key對應一個左邊的子節點
                    curr = curr->children[i];
                    found = true;
                    break;
                }
            }
            if (!found) curr = curr->children.back(); // 最右邊
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

class AVLTree {
private:
    AVLNode* root;

    int getHeight(AVLNode* n) { return n ? n->height : 0; }
    int getBalance(AVLNode* n) { return n ? getHeight(n->left) - getHeight(n->right) : 0; }

    // 右旋轉
    AVLNode* rightRotate(AVLNode* y) {
        AVLNode* x = y->left;
        AVLNode* T2 = x->right;
        x->right = y;
        y->left = T2;
        y->height = std::max(getHeight(y->left), getHeight(y->right)) + 1;
        x->height = std::max(getHeight(x->left), getHeight(x->right)) + 1;
        return x;
    }

    // 左旋轉
    AVLNode* leftRotate(AVLNode* x) {
        AVLNode* y = x->right;
        AVLNode* T2 = y->left;
        y->left = x;
        x->right = T2;
        x->height = std::max(getHeight(x->left), getHeight(x->right)) + 1;
        y->height = std::max(getHeight(y->left), getHeight(y->right)) + 1;
        return y;
    }

    AVLNode* insert(AVLNode* node, std::string name, int id) {
        if (node == nullptr) return new AVLNode(name, id);

        // [重點 1] 必須先處理重複，且名稱必須完全一致
        if (name == node->schoolName) {
            node->idList.push_back(id);
            return node;
        }

        if (name < node->schoolName)
            node->left = insert(node->left, name, id);
        else
            node->right = insert(node->right, name, id);

        node->height = 1 + std::max(getHeight(node->left), getHeight(node->right));

        int balance = getBalance(node);

        // [重點 2] 旋轉判定時，比較的對象必須是 node->left->schoolName
        // LL Case
        if (balance > 1 && name < node->left->schoolName)
            return rightRotate(node);

        // RR Case
        if (balance < -1 && name > node->right->schoolName)
            return leftRotate(node);

        // LR Case
        if (balance > 1 && name > node->left->schoolName) {
            node->left = leftRotate(node->left);
            return rightRotate(node);
        }

        // RL Case
        if (balance < -1 && name < node->right->schoolName) {
            node->right = rightRotate(node->right);
            return leftRotate(node);
        }

        return node;
    }

    int countNodes(AVLNode* node) {
        if (!node) return 0;
        return 1 + countNodes(node->left) + countNodes(node->right);
    }

public:
    AVLTree() : root(nullptr) {}

    void insertItem(std::string name, int id) {
        root = insert(root, name, id); // 必須賦值給 root，否則旋轉後 root 會指向舊位置
    }

    void ShowRootData(const std::vector<Data>& datalist) {
        if (!root) return;
        
        // 題目要求：節點內資料依序號由小到大顯示
        std::sort(root->idList.begin(), root->idList.end());

        std::cout << "Tree height = " << getHeight(root) << "\n";
        std::cout << "Number of nodes = " << countNodes(root) << "\n";

        for (int i = 0; i < root->idList.size(); ++i) {
            const Data& d = datalist[root->idList[i] - 1];
            std::cout << i + 1 << ": [" << d.serial << "] " 
                    << d.school_name << ", " << d.dept_name << ", "
                    << d.day_type << ", " << d.level << ", "
                    << d.students << ", " << d.graduates << "\n";
        }
        std::cout << "\n" << std::endl;
    }
};

int ParseNumber(std::string temp) {
    std::string num = "";
    for (int i = 0; i < temp.size(); i++) {
        if (temp[i] != '"' && temp[i] != ',') { // 排除'"' and ','
            num += temp[i];
        }
    }

    int result;
    try {
        result = std::stoi(num);
    } catch (const std::invalid_argument& exception) {
        result = 0;
    } catch (const std::out_of_range& exception) {
        result = 0;
    }

    return result;
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
            std::cin.clear(); // 重設錯誤狀態位元(cin)，如果不先cin.clear()那麼cin不會運作
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // 清除緩衝區中的殘留字元
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

    AVLTree AVL;
    for (int i = 0; i < datalist.size(); i++) {
        AVL.insertItem(datalist[i].school_name, datalist[i].serial);
    }
    AVL.ShowRootData(datalist);
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
    int command;
    std::vector<Data> datalist;
    while(true) {
        std::cout << "* Data Structures and Algorithms *" << std::endl;
        std::cout << "****** Balanced Search Tree ******" << std::endl;
        std::cout << "* 0. QUIT                        *" << std::endl;
        std::cout << "* 1. Build 23 tree               *" << std::endl;
        std::cout << "* 2. Build AVL tree              *" << std::endl;
        std::cout << "**********************************" << std::endl;
        std::cout << "Input a choice(0, 1, 2, 3, 4): ";
        std::cin >> command;
        std::cout << std::endl;

        if (command == 0) {
            return;
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
    }
}

int main() {
    LetsGo();
    return 0;
}
