/*11327117 黃郁婷 11327143 廖宇晨*/
#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>      // setw
#include <sstream>      // istringstream
#include <cmath>        // log
#include <algorithm>    // sort
#include <limits>       // numeric_limits

struct Data {
    int serial;
    std::string school_code;
    std::string school_name;
    std::string dept_code;
    std::string dept_name;
    std::string day_type;      // 日間/進修別
    std::string level;         // 等級別
    int students;
    int aboriginal;
    int graduates;             // 畢業生人數 -> 2-3 tree key
    std::string city;          // 縣市名稱
    std::string system;        // 體制別
};

struct Record {
    int serial;
    int graduate;
};

struct Node {
    bool isLeaf;
    std::vector<int> keys; // 存放畢業生人數（唯一值，從小到大）
    std::vector<std::vector<int>> idLists; // 存放每個 key 對應的原始資料序號列表
    std::vector<Node*> children;
    Node* parent;

    Node(Node* p, bool leaf) {
        parent = p;
        isLeaf = leaf; 
    }
};

struct AVLNode {
    std::string schoolName;    // 鍵值 (Key)：學校名稱
    std::vector<int> idList;   // 序號列表：存放所有該校系所的原始序號 (Serial)
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

    int findInsertPos(const std::vector<int>& keys, int target) { 
        int low = 0;
        int high = keys.size();
        while (low < high) { // 二元搜尋
            int mid = low + (high - low) / 2;
            if (keys[mid] < target) {
                low = mid + 1;
            } else {
                high = mid;
            }
        }
        return low;
    }

    void split(Node* node) { // 分裂節點，保持 2-3 樹的特性
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
            // 將中間值推入父節點 p
            int pos = findInsertPos(p->keys, midKey);

            p->keys.insert(p->keys.begin() + pos, midKey);
            p->idLists.insert(p->idLists.begin() + pos, midIdList);

            // 更新父節點的子節點指標
            p->children.erase(p->children.begin() + pos); 
            p->children.insert(p->children.begin() + pos, node1);
            p->children.insert(p->children.begin() + pos + 1, node2); 

            // 遞迴檢查父節點是否需要分裂
            if (p->keys.size() == 3) split(p);
        }
        delete node; // 刪除舊節點
    }

    int GetHeight(Node* node) {
        if (node == nullptr) return 0;
        if (node->isLeaf) return 1;
        return 1 + GetHeight(node->children[0]);
    }

    int CountNodes(Node* node) {
        if (node == nullptr) return 0;
        int count = 1; 
        if (!node->isLeaf) {
            for (Node* child : node->children) {
                count += CountNodes(child);
            }
        }
        return count;
    }

  public:
    TwoThreeTree() : root(nullptr) {}

    void insertItem(int gradCount, int id) {
        if (root == nullptr) {
            root = new Node(nullptr, true);
            root->keys.push_back(gradCount);
            root->idLists.push_back({id});
            return;
        }

        // 1. 尋找目標葉子節點
        Node* curr = root;
        while (!curr->isLeaf) {
            bool found = false;
            for (int i = 0; i < curr->keys.size(); ++i) {
                if (gradCount == curr->keys[i]) { // 數值已存在，加入序號
                    curr->idLists[i].push_back(id);
                    return;
                }
                if (gradCount < curr->keys[i]) {
                    curr = curr->children[i];
                    found = true;
                    break;
                }
            }
            if (!found) curr = curr->children.back(); 
        }

        // 2. 檢查葉子節點中是否已有相同的 Key
        for (int i = 0; i < curr->keys.size(); ++i) {
            if (curr->keys[i] == gradCount) {
                curr->idLists[i].push_back(id);
                return;
            }
        }

        // 3. 插入新 Key 並排序
        curr->keys.push_back(gradCount);
        curr->idLists.push_back({id});

        for (int i = curr->keys.size() - 1; i > 0; i--) {
            if (curr->keys[i] < curr->keys[i-1]) {
                std::swap(curr->keys[i], curr->keys[i-1]);
                std::swap(curr->idLists[i], curr->idLists[i-1]);
            } else {
                break;
            }
        }

        // 4. 若節點溢位（3個Key），執行分裂
        if (curr->keys.size() == 3) {
            split(curr);
        }
    }

    void ShowRootData(std::vector<Data> &datalist) {
        if (!root) return;
        int count = 1;
        std::cout << "Tree height = " << GetHeight(root) << "\n";
        std::cout << "Number of nodes = " << CountNodes(root) << "\n";
        for (int i = 0; i < root->idLists.size(); i++) {
            for (int j = 0; j < root->idLists[i].size(); j++) {
                const Data& d = datalist[root->idLists[i][j] - 1];
                std::cout << count << ": [" << d.serial << "] "
                          << d.school_name << ", " << d.dept_name << ", "
                          << d.day_type << ", " << d.level << ", "
                          << d.students << ", " << d.graduates << "\n";
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
        root = insert(root, name, id);
    }

    void ShowRootData(const std::vector<Data>& datalist) {
        if (!root) {
            std::cout << "Tree is empty." << std::endl;
            return;
        }
        
        std::cout << "Tree height = " << getHeight(root) << "\n";
        std::cout << "Number of nodes = " << countNodes(root) << "\n";

        std::vector<int> sortedIds = root->idList;
        std::sort(sortedIds.begin(), sortedIds.end());

        for (int i = 0; i < sortedIds.size(); ++i) {
            const Data& d = datalist[sortedIds[i] - 1];
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
        if (temp[i] != '"' && temp[i] != ',') { 
            num += temp[i];
        }
    }
    int result;
    try {
        result = std::stoi(num);
    } catch (...) {
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
        if (name == "0") return false;
        
        filename = "input" + name + ".txt";
        std::ifstream inputFile(filename);
        if (!inputFile.is_open()) {
            std::cout << "\n### " << filename << " does not exist! ###" << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        } else {
            inputFile.close();
            break;
        }
    }

    std::ifstream inputFile(filename);
    std::string line;
    std::string temp;
    int i = 1;
    // 跳過前三行標頭
    std::getline(inputFile, line);
    std::getline(inputFile, line);
    std::getline(inputFile, line); 

    while (std::getline(inputFile, line)) {
        Data newdata;
        newdata.serial = i++;
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
    if (!ReadFile(datalist)) return;
    TwoThreeTree Tree23;
    for (int i = 0; i < datalist.size(); i++) {
        Tree23.insertItem(datalist[i].graduates, datalist[i].serial);
    }
    Tree23.ShowRootData(datalist);
}

void Mission2(std::vector<Data> &datalist) {
    if (datalist.empty()) {
        std::cout << "\n### Choose 1 first. ###\n" << std::endl;
        return;
    }
    AVLTree AVL;
    for (int i = 0; i < datalist.size(); i++) {
        AVL.insertItem(datalist[i].school_name, datalist[i].serial);
    }
    AVL.ShowRootData(datalist);
}

void LetsGo() {
    int command;
    std::vector<Data> datalist;
    while(true) {
        std::cout << "* Data Structures and Algorithms *" << std::endl;
        std::cout << "****** Balanced Search Tree ******" << std::endl;
        std::cout << "* 0. QUIT                         *" << std::endl;
        std::cout << "* 1. Build 23 tree                *" << std::endl;
        std::cout << "* 2. Build AVL tree               *" << std::endl;
        std::cout << "**********************************" << std::endl;
        std::cout << "Input a choice(0, 1, 2): ";
        if (!(std::cin >> command)) break;

        if (command == 0) break;
        else if (command == 1) {
            datalist.clear();
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
