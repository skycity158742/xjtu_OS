#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CHAR 256

typedef struct linklist_node{
    int c; // the character
    int freq; // frequency
    struct linklist_node* next;
} lknode, linklist;  

// Huffman树节点定义
typedef struct HuffmanTreeNode {
    int c;       // character
    int freq;    // frequency
    struct HuffmanTreeNode* left;
    struct HuffmanTreeNode* right;
} HuffmanTreeNode;

// 创建新的Huffman树节点
HuffmanTreeNode* createHuffmanNode(int c, int freq) {
    HuffmanTreeNode* node = (HuffmanTreeNode*)malloc(sizeof(HuffmanTreeNode));
    node->c = c;
    node->freq = freq;
    node->left = NULL;
    node->right = NULL;
    return node;
}

// 创建优先队列节点
typedef struct MinHeapNode {
    HuffmanTreeNode* treeNode;
    struct MinHeapNode* next;
} MinHeapNode;

// 创建新的优先队列节点
MinHeapNode* createMinHeapNode(HuffmanTreeNode* treeNode) {
    MinHeapNode* node = (MinHeapNode*)malloc(sizeof(MinHeapNode));
    node->treeNode = treeNode;
    node->next = NULL;
    return node;
}

// 插入Huffman树节点到优先队列
void insertMinHeap(MinHeapNode** head, HuffmanTreeNode* treeNode) {
    MinHeapNode* newNode = createMinHeapNode(treeNode);
    if (*head == NULL || (*head)->treeNode->freq > treeNode->freq) {
        newNode->next = *head;
        *head = newNode;
    } else {
        MinHeapNode* current = *head;
        while (current->next != NULL && current->next->treeNode->freq <= treeNode->freq) {
            current = current->next;
        }
        newNode->next = current->next;
        current->next = newNode;
    }
}

// 从优先队列中取出最小频率的Huffman树节点
HuffmanTreeNode* extractMin(MinHeapNode** head) {
    if (*head == NULL) {
        return NULL;
    }
    MinHeapNode* temp = *head;
    *head = (*head)->next;
    HuffmanTreeNode* treeNode = temp->treeNode;
    free(temp);
    return treeNode;
}

// 建立Huffman树
HuffmanTreeNode* buildHuffmanTree(linklist* head) {
    MinHeapNode* minHeap = NULL;
    linklist* current = head;
    current = current->next;
    
    // 创建初始优先队列
    while (current != NULL) {
        insertMinHeap(&minHeap, createHuffmanNode(current->c, current->freq));
        current = current->next;
    }
    
    // 建立Huffman树
    while (minHeap != NULL && minHeap->next != NULL) {
        HuffmanTreeNode* left = extractMin(&minHeap);
        HuffmanTreeNode* right = extractMin(&minHeap);
        HuffmanTreeNode* newNode = createHuffmanNode(-1, left->freq + right->freq);
        newNode->left = left;
        newNode->right = right;
        insertMinHeap(&minHeap, newNode);
    }
    
    return extractMin(&minHeap);
}

// 打印Huffman编码
void printHuffmanCodes(HuffmanTreeNode* root, int* code, int top) {
    if (root->left) {
        code[top] = 0;
        printHuffmanCodes(root->left, code, top + 1);
    }
    if (root->right) {
        code[top] = 1;
        printHuffmanCodes(root->right, code, top + 1);
    }
    if (root->left == NULL && root->right == NULL) {
        printf("%c: ", root->c);
        for (int i = 0; i < top; ++i) {
            printf("%d", code[i]);
        }
        printf("\n");
    }
}

// 生成Huffman编码表
void generateCodes(HuffmanTreeNode* root, int* code, int top, char codes[MAX_CHAR][MAX_CHAR]) {
    if (root->left) {
        code[top] = 0;
        generateCodes(root->left, code, top + 1, codes);
    }
    if (root->right) {
        code[top] = 1;
        generateCodes(root->right, code, top + 1, codes);
    }
    if (root->left == NULL && root->right == NULL) {
        for (int i = 0; i < top; ++i) {
            codes[root->c][i] = code[i] + '0';
        }
        codes[root->c][top] = '\0';
    }
}

// 将输入文件转换为Huffman编码后的二进制码流
void encodeFile(const char* inputFilename, const char* outputFilename, char codes[MAX_CHAR][MAX_CHAR]) {
    FILE *inputFile = fopen(inputFilename, "r");
    if (inputFile == NULL) {
        perror("Error opening input file");
        exit(1);
    }
    
    FILE *outputFile = fopen(outputFilename, "wb");
    if (outputFile == NULL) {
        perror("Error opening output file");
        exit(1);
    }

    char ch;
    while ((ch = fgetc(inputFile)) != EOF) {
        fprintf(outputFile, "%s", codes[(unsigned char)ch]);
    }

    fclose(inputFile);
    fclose(outputFile);
}

// 解码二进制码流文件到文本文件
void decodeFile(const char* inputFilename, const char* outputFilename, HuffmanTreeNode* root) {
    FILE *inputFile = fopen(inputFilename, "rb");
    if (inputFile == NULL) {
        perror("Error opening input file");
        exit(1);
    }

    FILE *outputFile = fopen(outputFilename, "w");
    if (outputFile == NULL) {
        perror("Error opening output file");
        exit(1);
    }

    HuffmanTreeNode* current = root;
    char bit;
    while ((bit = fgetc(inputFile)) != EOF) {
        if (bit == '0') {
            current = current->left;
        } else if (bit == '1') {
            current = current->right;
        }

        if (current->left == NULL && current->right == NULL) {
            fputc(current->c, outputFile);
            current = root;
        }
    }

    fclose(inputFile);
    fclose(outputFile);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }
    
    // create freq table
    int frequency[MAX_CHAR] = {0};
    countFrequency(file, frequency);
    printFrequency(frequency);
    
    // change to linklist(sorted)
    linklist* linklist1 = build_linklist(frequency);
    print_the_list(linklist1);
    
    // build Huffman tree
    HuffmanTreeNode* huffmanTree = buildHuffmanTree(linklist1);

    // 生成Huffman编码表
    int code[100], top = 0;
    char codes[MAX_CHAR][MAX_CHAR] = { {0} };
    generateCodes(huffmanTree, code, top, codes);

    // 打印Huffman编码
    for (int i = 0; i < MAX_CHAR; ++i) {
        if (codes[i][0] != '\0') {
            printf("%c: %s\n", i, codes[i]);
        }
    }

    // 将输入文件转换为Huffman编码后的二进制码流
    encodeFile(argv[1], "output.bin", codes);

    // 将Huffman编码的二进制码流解码为文本文件
    decodeFile("output.bin", "output1.txt", huffmanTree);

    free_the_list(linklist1);
    fclose(file);
    return 0;
}
