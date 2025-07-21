#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CHAR 1024

typedef struct linklist_node{
    int c; // the character
    int freq; // frequency
    struct linklist_node* next;
}lknode, linklist;  

// Huffman tree node
typedef struct HuffmanTreeNode {
    int c;       // character
    int freq;    // frequency
    struct HuffmanTreeNode* left;
    struct HuffmanTreeNode* right;
} HuffmanTreeNode;

// create new tree node 
HuffmanTreeNode* createHuffmanNode(int c, int freq) {
    HuffmanTreeNode* node = (HuffmanTreeNode*)malloc(sizeof(HuffmanTreeNode));
    node->c = c;
    node->freq = freq;
    node->left = NULL;
    node->right = NULL;
    return node;
}

// priority queue node(min heap)
typedef struct MinHeapNode {
    HuffmanTreeNode* treeNode;
    struct MinHeapNode* next;
} MinHeapNode;

// create new queue node
MinHeapNode* createMinHeapNode(HuffmanTreeNode* treeNode) {
    MinHeapNode* node = (MinHeapNode*)malloc(sizeof(MinHeapNode));
    node->treeNode = treeNode;
    node->next = NULL;
    return node;
}

// insert into min heap
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

// find min tree in the forest
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

// build Huffman tree
HuffmanTreeNode* buildHuffmanTree(linklist* head) {
    MinHeapNode* minHeap = NULL;
    linklist* current = head;
    current = current->next;
    
    // init priority queue
    while (current != NULL) {
        insertMinHeap(&minHeap, createHuffmanNode(current->c, current->freq));
        current = current->next;
    }
    
    // build Huffman tree
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

// printf Huffman code
void printHuffmanCodes(HuffmanTreeNode* root, int* code, int top) {
    if(top == 0) printf("Here is the Huffman table:\n");
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


linklist* build_linklist(int* f){
    int i, j; 
    int tmp, tmpj;
    int count = 0;
    // create linklist
    linklist* ll = (linklist*)malloc(sizeof(linklist));
    lknode* tail = ll;
    for(i = 0; i < MAX_CHAR; i++){
        if(f[i] != 0){
            count++;
        }
    }
    
    // pop sort, needs modify to another sort algo
    for(i = 0; i < count; i++) {
        tmp = 0;
        tmpj = 0;
        for(j = 0; j < MAX_CHAR; j++){//find max
            if(tmp < f[j]) {
                tmp = f[j];
                tmpj = j;
            }
        }
        
        // maxfreq found, add it to linklist
        lknode* node = (lknode*)malloc(sizeof(lknode));
        if(node == NULL) 
            exit(EXIT_FAILURE);
        node->next = NULL;
        // printf("new node addr: %ld", &node);
        node->c = tmpj;
        node->freq = f[tmpj];
        tail->next = node;
        tail = node;
        // printf(" adding this node: %c -> %d\n", node->c, node->freq);
        
        // set 0 to f[j]
        f[tmpj] = 0;
    }
    return ll;
}

void print_the_list(linklist* lk){
    linklist* l = lk;
    l = l->next;
    while(l != NULL){
        printf("%c -> %d\n", l->c, l->freq);
        l = l->next;
    }
}

void free_the_list(linklist* lk) {
    linklist *l = lk, *l1;
    while(l!=NULL) {
        l1 = l->next;
        free(l);
        l = l1;
    }
}

void countFrequency(FILE *file, int *frequency) {
    char ch;
    while ((ch = fgetc(file)) != EOF) {
        frequency[(unsigned char)ch]++;
    }
}

void printFrequency(int *frequency) {
    for (int i = 0; i < MAX_CHAR; i++) {
        if (frequency[i] > 0) {
            printf("%c -> %d\n", i, frequency[i]);
        }
    }
}

// make Huffman code table
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

// encode the file to binary stream.
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

// decode binary stream to file
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

int main() {
    
    FILE *file = fopen("input.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }
    
    // create freq table
    int frequency[MAX_CHAR] = {0};
    countFrequency(file, frequency);
    printf("*********************************************************");
    printf("\nHere is the frequency table:\n");
    printFrequency(frequency);
    
    // change to linklist(sorted)
    linklist* linklist1 = build_linklist(frequency);
    // print_the_list(linklist1);
    
    // build Huffman tree
    HuffmanTreeNode* huffmanTree = buildHuffmanTree(linklist1);
    printf("\n*********************************************************\n");
    printf("\nHuffman tree build finished! Starting to make Huffman table.\n");
    printf("\n*********************************************************\n");
    int code[100], top = 0;
    printHuffmanCodes(huffmanTree, code, top);
    char codes[MAX_CHAR][MAX_CHAR] = { {0} };
    generateCodes(huffmanTree, code, top, codes);
    
    // encode input.txt to binary stream 
    encodeFile("input.txt", "output.bin", codes);
    printf("\n*********************************************************\n");
    printf("Encode finished! Check it in output.bin");
    
    // decode binary stream to file
    decodeFile("output.bin", "output1.txt", huffmanTree);
    printf("\n*********************************************************\n");
    printf("Decode finished! Check it in output1.txt");
    
    // check if the encoding and decoding is correct.
    // todo
    // check_correctness(input.txt, output.txt);

    free_the_list(linklist1);
    fclose(file);
    return 0;
}
