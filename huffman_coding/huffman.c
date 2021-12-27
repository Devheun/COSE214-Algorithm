#define BINARY_MODE //주석처리하면 text 인코딩

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef struct Node
{
    unsigned char   data;   // 문자   
    int      freq;          // 빈도
    struct   Node* left;      // 왼쪽 서브트리 포인터
    struct   Node* right;    // 오른쪽 서브트리 포인터
} tNode;

////////////////////////////////////////////////////////////////////////////////
typedef struct
{
    int   last;      // 힙에 저장된 마지막 element의 index
    int   capacity;   // heapArr의 크기 = 256
    tNode** heapArr;
} HEAP;

// 힙 생성
// 배열을 위한 메모리 할당 (capacity)
// last = -1
HEAP* heapCreate(int capacity); // 256

// 최소힙 유지
static void _reheapUp(HEAP* heap, int index);

// 힙에 원소 삽입
// _reheapUp 함수 호출
int heapInsert(HEAP* heap, tNode* data);

// 최소힙 유지
static void _reheapDown(HEAP* heap, int index);

// 최소값 제거
// _reheapDown 함수 호출
tNode* heapDelete(HEAP* heap);

// 힙 메모리 해제
void heapDestroy(HEAP* heap);

////////////////////////////////////////////////////////////////////////////////
// 파일에 속한 각 문자(바이트)의 빈도 저장
// return value : 파일에서 읽은 바이트 수
int read_chars(FILE* fp, int ch_freq[]); //입력된 파일로 문자를 하나하나 읽으면서 각 문자마다 빈도값 저장

// 허프만 코드에 대한 메모리 해제
void free_huffman_code(char* codes[]); //recursively하게 순회하면서 메모리해제

// 새로운 노드를 생성
// 좌/우 subtree가 NULL이고 문자(data)와 빈도(freq)가 저장됨
// return value : 노드의 포인터
tNode* newNode(unsigned char data, int freq); //문자별로 하나씩 노드 다 만들때도 쓰인다. 
//두 서브트리를 결합해서 부모노드가 하나 생길때도 쓰임(메모리 할당하고 그 안에 데이터 - 서브트리들의 빈도값 합한거, 데이터부분엔 아무거나)
//left right 포인터 첨에 NULL로 초기화

// 허프만 트리를 생성
// 1. capacity 256 짜리 빈(empty) 힙 생성
// 2. 개별 알파벳에 대한 노드 생성
// 3. 힙에 삽입 (minheap 구성)
// 4. 2개의 최소값을 갖는 트리 추출
// 5. 두 트리를 결합 후 새 노드에 추가
// 6. 새 트리를 힙에 삽입
// 7. 힙에 한개의 노드가 남을 때까지 반복 //heap->last==0이면 한개 남은 거
// return value: 트리의 root 노드의 포인터
tNode* make_huffman_tree(int ch_freq[]); //여기 함수에 힙과 관련된게 다 사용됨

// 허프만 트리를 순회하며 허프만 코드를 생성하여 codes에 저장
// leaf 노드에서만 코드를 생성
// strdup 함수 사용함
// make_huffman_code 함수에서 호출
void traverse_tree(tNode* root, char* code, int depth, char* codes[]);

// 트리 메모리 해제
void destroyTree(tNode* root);

// 입력 텍스트 파일(infp)을 허프만 코드를 이용하여 출력 파일(outfp)로 인코딩
// return value : 인코딩된 파일의 바이트 수
int encoding(char* codes[], FILE* infp, FILE* outfp);
int encoding_binary(char* codes[], FILE* infp, FILE* outfp);

// 입력 파일(infp)을 허프만 트리를 이용하여 텍스트 파일(outfp)로 디코딩
void decoding(tNode* root, FILE* infp, FILE* outfp);
void decoding_binary(tNode* root, FILE* infp, FILE* outfp);

/*
getc
fwrite
fread
fseek
ftell
비트연산
비트단위로 쓸수없어서 한 바이트 단위로, 8개의 비트가 되면 파일에 출력해야함
8의 배수로 안나눠떨어질때, 파일의 맨 끝에 integer값 하나를 기록해둠
->몇개의 비트를 읽어야할지 (나중에 decoding할때 읽어들일때
맨 마지막 integer 먼저 읽어서
몇개 비트를 읽어야하는가를 미리 알아낸다)
*/

////////////////////////////////////////////////////////////////////////////////
// 문자별 빈도 출력 (for debugging)
void print_char_freq(int ch_freq[])
{
    int i;

    for (i = 0; i < 256; i++)
    {
        printf("%d\t%d\n", i, ch_freq[i]); // 문자인덱스, 빈도
    }
}

////////////////////////////////////////////////////////////////////////////////
// 허프만 트리로부터 허프만 코드를 생성
// traverse_tree 함수 호출
void make_huffman_code(tNode* root, char* codes[]) //트리 순회해가면서 왼쪽으로 갈땐 코드 0을 저장,오른쪽갈땐 코드 1 갖도록 -> 리프노드까지(리프노드에 문자에 대한 코드는 그동안 모아놨던 코드들)
{
    char code[256];

    traverse_tree(root, code, 0, codes);
}

////////////////////////////////////////////////////////////////////////////////
// 허프만 코드를 화면에 출력
void print_huffman_code(char* codes[])
{
    int i;

    for (i = 0; i < 256; i++)
    {
        printf("%d\t%s\n", i, codes[i]);
    }
}

////////////////////////////////////////////////////////////////////////////////
// argv[1] : 입력 텍스트 파일
// argv[2] : encoded 파일
// argv[3] : decoded 파일
int main(int argc, char** argv)
{
    FILE* fp;
    FILE* infp, * outfp;
    int ch_freq[256] = { 0, }; // 문자별 빈도
    char* codes[256]; // 문자별 허프만 코드 (ragged 배열)
    tNode* huffman_tree; // 허프만 트리

    if (argc != 4)
    {
        fprintf(stderr, "%s input-file encoded-file decoded-file\n", argv[0]);
        return 1;
    }

    ////////////////////////////////////////
    // 입력 텍스트 파일
    fp = fopen(argv[1], "rt");
    if (fp == NULL)
    {
        fprintf(stderr, "Error: cannot open file [%s]\n", argv[1]);
        return 1;
    }

    // 텍스트 파일로부터 문자별 빈도 저장
    int num_bytes = read_chars(fp, ch_freq);

    fclose(fp);

    // 문자별 빈도 출력 (for debugging)
    //print_char_freq( ch_freq);

    // 허프만 트리 생성
    huffman_tree = make_huffman_tree(ch_freq);

    // 허프만 코드 생성
    make_huffman_code(huffman_tree, codes);

    // 허프만 코드 출력 (stdout)
    print_huffman_code(codes);

    ////////////////////////////////////////
    // 입력: 텍스트 파일
    infp = fopen(argv[1], "rt");

#ifdef BINARY_MODE
    // 출력: 바이너리 코드
    outfp = fopen(argv[2], "wb");
#else
    // 출력: 텍스트 코드
    outfp = fopen(argv[2], "wt");
#endif

    // 허프만코드를 이용하여 인코딩(압축)
#ifdef BINARY_MODE
    int encoded_bytes = encoding_binary(codes, infp, outfp);
#else
    int encoded_bytes = encoding(codes, infp, outfp);
#endif

    // 허프만 코드 메모리 해제
    free_huffman_code(codes);

    fclose(infp);
    fclose(outfp);

    ////////////////////////////////////////
    // 입력: 바이너리 코드
#ifdef BINARY_MODE
    infp = fopen(argv[2], "rb");
#else
    infp = fopen(argv[2], "rt");
#endif

    // 출력: 텍스트 파일
    outfp = fopen(argv[3], "wt");

    // 허프만 트리를 이용하여 디코딩
#ifdef BINARY_MODE
    decoding_binary(huffman_tree, infp, outfp);
#else
    decoding(huffman_tree, infp, outfp);
#endif

    // 허프만 트리 메모리 해제
    destroyTree(huffman_tree);

    fclose(infp);
    fclose(outfp);

    ////////////////////////////////////////
    printf("# of bytes of the original text = %d\n", num_bytes);
    printf("# of bytes of the compressed text = %d\n", encoded_bytes);
    printf("compression ratio = %.2f\n", ((float)num_bytes - encoded_bytes) / num_bytes * 100);

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// 힙의 내용 출력 (for debugging)
void heapPrint(HEAP* heap)
{
    int i;
    tNode** p = heap->heapArr;
    int last = heap->last;

    for (i = 0; i <= last; i++)
    {
        printf("[%d]%c(%6d)\n", i, p[i]->data, p[i]->freq);
    }
    printf("\n");
}

////////////////////////////////////////////////////////////////////////////////
// 힙 생성
// 배열을 위한 메모리 할당 (capacity)
// last = -1
HEAP* heapCreate(int capacity)
{
    HEAP* heap;

    heap = (HEAP*)malloc(sizeof(HEAP));
    if (!heap) return NULL;

    heap->last = -1;
    heap->capacity = capacity;
    heap->heapArr = (tNode**)malloc(sizeof(tNode*) * capacity);
    if (heap->heapArr == NULL)
    {
        fprintf(stderr, "Error : not enough memory!\n");
        free(heap);
        return NULL;
    }
    return heap;
}

////////////////////////////////////////////////////////////////////////////////
// 최소힙 유지
static void _reheapUp(HEAP* heap, int index)
{
    tNode** arr = heap->heapArr;
    int parent;

    while (1)
    {
        if (index == 0) return; // root node

        parent = (index - 1) / 2;

        if (arr[index]->freq < arr[parent]->freq) // exchange (for minheap)
        {
            tNode* temp = arr[index];
            arr[index] = arr[parent];
            arr[parent] = temp;

            index = parent;
        }
        else return;
    }
}

////////////////////////////////////////////////////////////////////////////////
// 힙에 원소 삽입
// _reheapUp 함수 호출
int heapInsert(HEAP* heap, tNode* data)
{
    if (heap->last == heap->capacity - 1)
        return 0;

    (heap->last)++;
    (heap->heapArr)[heap->last] = data;

    _reheapUp(heap, heap->last);

    return 1;
}

////////////////////////////////////////////////////////////////////////////////
// 최소힙 유지
static void _reheapDown(HEAP* heap, int index)
{
    tNode** arr = heap->heapArr;
    tNode* leftData;
    tNode* rightData;
    int noright = 0;
    int largeindex; // index of left or right child with large key

    while (1)
    {
        if ((index * 2 + 1) > heap->last) return; // leaf node (there is no left subtree)

        leftData = arr[index * 2 + 1];
        if (index * 2 + 2 <= heap->last) rightData = arr[index * 2 + 2];
        else noright = 1;

        if (noright || leftData->freq < rightData->freq) largeindex = index * 2 + 1; // left child
        else largeindex = index * 2 + 2; // right child

        if (arr[index]->freq > arr[largeindex]->freq) // exchange (for minheap)
        {
            tNode* temp = arr[index];
            arr[index] = arr[largeindex];
            arr[largeindex] = temp;

            index = largeindex;

            noright = 0;
        }
        else return;
    }
}

////////////////////////////////////////////////////////////////////////////////
// 최소값 제거
// _reheapDown 함수 호출
tNode* heapDelete(HEAP* heap)
{
    if (heap->last == -1) return NULL; // empty heap

    tNode* data = heap->heapArr[0];
    heap->heapArr[0] = heap->heapArr[heap->last];

    (heap->last)--;

    _reheapDown(heap, 0);

    return data;
}

////////////////////////////////////////////////////////////////////////////////
// 힙 메모리 해제
void heapDestroy(HEAP* heap)
{
    free(heap->heapArr);
    free(heap);
}

// 파일에 속한 각 문자(바이트)의 빈도 저장
// return value : 파일에서 읽은 바이트 수
int read_chars(FILE* fp, int ch_freq[]) { //입력된 파일로 문자를 하나하나 읽으면서 각 문자마다 빈도값 저장

    int character;
    int cnt = 0;

    while ((character = fgetc(fp)) != EOF) {

        if (feof(fp) != 0)break; //파일 끝일때 탈출해야하니
        ch_freq[character]++;
        cnt++;
    }

    return cnt;
}

// 허프만 트리를 생성
// 1. capacity 256 짜리 빈(empty) 힙 생성
// 2. 개별 알파벳에 대한 노드 생성
// 3. 힙에 삽입 (minheap 구성)
// 4. 2개의 최소값을 갖는 트리 추출
// 5. 두 트리를 결합 후 새 노드에 추가
// 6. 새 트리를 힙에 삽입
// 7. 힙에 한개의 노드가 남을 때까지 반복 //heap->last==0이면 한개 남은 거
// return value: 트리의 root 노드의 포인터
tNode* make_huffman_tree(int ch_freq[]) { //여기 함수에 힙과 관련된게 다 사용됨
    HEAP* heap = heapCreate(256);

    for (int i = 0; i < 256; i++) {
        heapInsert(heap, newNode(i, ch_freq[i]));
    }

    while (heap->last > 0) { //heap->last==0이면 한개남은거니 >0까지 반복
        tNode* first = heapDelete(heap);
        tNode* second = heapDelete(heap);

        tNode* parent = newNode(255, first->freq + second->freq);//데이터 아무거나 넣어줘도 됨.
        parent->left = first;
        parent->right = second;
        heapInsert(heap, parent);

    }

    tNode* root;
    root = heapDelete(heap);
    //heap 다 썼으니 필요없다. destroy해주어야함
    heapDestroy(heap);

    return root;

}

// 새로운 노드를 생성
// 좌/우 subtree가 NULL이고 문자(data)와 빈도(freq)가 저장됨
// return value : 노드의 포인터
tNode* newNode(unsigned char data, int freq) { //문자별로 하나씩 노드 다 만들때도 쓰인다. 
//두 서브트리를 결합해서 부모노드가 하나 생길때도 쓰임(메모리 할당하고 그 안에 데이터 - 서브트리들의 빈도값 합한거, 데이터부분엔 아무거나)
//left right 포인터 첨에 NULL로 초기화

    tNode* node;
    node = (tNode*)malloc(sizeof(tNode));

    node->left = NULL;
    node->right = NULL;
    node->data = data;
    node->freq = freq;

    return node;

}

// 허프만 트리를 순회하며 허프만 코드를 생성하여 codes에 저장
// leaf 노드에서만 코드를 생성
// strdup 함수 사용함
// make_huffman_code 함수에서 호출
//트리 순회해가면서 왼쪽으로 갈땐 코드 0을 저장,오른쪽갈땐 코드 1 갖도록
//->리프노드까지(리프노드에 문자에 대한 코드는 그동안 모아놨던 코드들)
void traverse_tree(tNode* root, char* code, int depth, char* codes[]) {

    if (root->left == NULL && root->right == NULL) {
        code[depth] = '\0'; //이거 꼭 해주어야 끝이라고 인식한다.
        codes[root->data] = strdup(code);
        return;
    }

    //왼쪽으로 갈때
    code[depth] = '0';
    traverse_tree(root->left, code, depth + 1, codes);

    //오른쪽으로 갈때
    code[depth] = '1';
    traverse_tree(root->right, code, depth + 1, codes);
}

// 입력 텍스트 파일(infp)을 허프만 코드를 이용하여 출력 파일(outfp)로 인코딩
// return value : 인코딩된 파일의 바이트 수
//교수님은 /8로 하셨다.
int encoding(char* codes[], FILE* infp, FILE* outfp) { //파일 압축하는거 ! decoding 했을 때 sample.txt와 같게 나와야함

    int cnt = 0;
    int character;

    while ((character = fgetc(infp)) != EOF) {

        //파일에다가 쓰는거
        fprintf(outfp, "%s", codes[character]);
        int length = strlen(codes[character]);
        cnt += length;
        if (feof(infp) != 0)break;
    }

    return (cnt / 8);
}

// 허프만 코드에 대한 메모리 해제
void free_huffman_code(char* codes[]) { //recursively하게 순회하면서 메모리해제

    for (int i = 0; i < 256; i++) {
        if (codes[i] != NULL) {
            free(codes[i]);
        }
    }
}

// 입력 파일(infp)을 허프만 트리를 이용하여 텍스트 파일(outfp)로 디코딩
void decoding(tNode* root, FILE* infp, FILE* outfp) {

    tNode* node;
    node = root;
    char character;

    while ((character = fgetc(infp)) != EOF) {

        if (character == '0') {
            node = node->left;
        }
        else if (character) {
            node = node->right;
        }

        if (node->left == NULL && node->right == NULL) {
            //여기도 마찬가지로 파일에다가 써줘야함
            fprintf(outfp, "%c", node->data);

            node = root; //이거 안해주면 segmentation 뜸
            //하나 끝까지 다 파고들었으면 다시 원상복귀 해줘야함
        }

        if (feof(infp) != 0)break;
    }
}

// 트리 메모리 해제
void destroyTree(tNode* root) {

    if (root != NULL) {

        if (root->left != NULL) {
            destroyTree(root->left);
        }
        if (root->right != NULL) {
            destroyTree(root->right);
        }
        free(root);
    }
}

/*
getc
fwrite
fread
fseek
ftell
비트연산
비트단위로 쓸수없어서 한 바이트 단위로, 8개의 비트가 되면 파일에 출력해야함
8의 배수로 안나눠떨어질때, 파일의 맨 끝에 integer값 하나를 기록해둠
->몇개의 비트를 읽어야할지 (나중에 decoding할때 읽어들일때
맨 마지막 integer 먼저 읽어서
몇개 비트를 읽어야하는가를 미리 알아낸다)
*/
int encoding_binary(char* codes[], FILE* infp, FILE* outfp) {

    int character;
    int bitcount = 0;
    int length = 0;
    char byte = 0;
    int bit = 0;
    char str[256];

    while ((character = fgetc(infp)) != EOF) {

        length = strlen(codes[character]);
        bit += length; //비트 수를 알아야 함

        strcpy(str, codes[character]);

        for (int i = 0; i < length; i++) {

            byte = byte << 1; //한쪽으로 옮겨주어야함 안그럼 겹침
            byte = (byte | (str[i] - '0'));
            bitcount++;
            if (bitcount == 8) { //8되면 무조건 써주어야함
                fwrite(&byte, sizeof(char), 1, outfp);
                bitcount = 0;
                byte = 0;
            }
        }
        if (feof(infp) != 0)break;
    }
    //bitcount가 8이 안되고 끝났을때 남은 것들 출력해야함.
    if (bitcount != 0) {
        //0000 1011이면 땡겨서 출력해야하니까 여기서도
        //shift연산 써줘야함
        byte = byte << (8 - bitcount);
        fwrite(&byte, sizeof(char), 1, outfp);
    }

    fwrite(&bitcount, sizeof(int), 1, outfp);//마지막 기록용도

    return bit / 8;
}

/*
getc
fwrite
fread
fseek 파일 위치 찾고 파일 위치의 포인터를 알아내는거
ftell
비트연산
비트단위로 쓸수없어서 한 바이트 단위로, 8개의 비트가 되면 파일에 출력해야함
8의 배수로 안나눠떨어질때, 파일의 맨 끝에 integer값 하나를 기록해둠
->몇개의 비트를 읽어야할지 (나중에 decoding할때 읽어들일때
맨 마지막 integer 먼저 읽어서
몇개 비트를 읽어야하는가를 미리 알아낸다)
*/
void decoding_binary(tNode* root, FILE* infp, FILE* outfp) {
    tNode* node;
    node = root;
    char bit;
    int check;
    int bitcount = 0;
    int end;

    fseek(infp, -4, SEEK_END);
    fread(&bitcount, sizeof(int), 1, infp);
    fseek(infp, -5, SEEK_END);
    end = ftell(infp);
    fseek(infp, 0, SEEK_SET);

    while (feof(infp) == 0) {
        fread(&bit, sizeof(char), 1, infp);
   
        for (int i = 0; i < 8; i++) {
            //1110 0110일때
            //1000 0000 만들어서 &취해주고 1000 0000이니까 왼쪽으로 가도록
            check = bit & (1 << (7 - i));

            if (check == 0) {
                node = node->left;
            }
            else if (check) {
                node = node->right;
            }

            if (node->left == NULL && node->right == NULL) {
                //여기도 마찬가지로 파일에다가 써줘야함
                fprintf(outfp, "%c", node->data);

                node = root; //이거 안해주면 segmentation 뜸
                //하나 끝까지 다 파고들었으면 다시 원상복귀 해줘야함
            }

        }
        if (ftell(infp) == end)break;
    }

    fread(&bit, sizeof(char), 1, infp);
    if (bitcount == 0) {
        for (int i = 0; i < 8; i++) {
            check = bit & (1 << (7 - i));

            if (check == 0) {
                node = node->left;
            }
            else if (check) {
                node = node->right;
            }

            if (node->left == NULL && node->right == NULL) {
                //여기도 마찬가지로 파일에다가 써줘야함
                fprintf(outfp, "%c", node->data);

                node = root; //이거 안해주면 segmentation 뜸
                //하나 끝까지 다 파고들었으면 다시 원상복귀 해줘야함
            }

        }
    }

    else {
    for (int i = 0; i < bitcount; i++) {
        check = bit & (1 << (7 - i));

        if (check == 0) {
            node = node->left;
        }
        else if (check) {
            node = node->right;
        }

        if (node->left == NULL && node->right == NULL) {
            //여기도 마찬가지로 파일에다가 써줘야함
            fprintf(outfp, "%c", node->data);

            node = root; //이거 안해주면 segmentation 뜸
            //하나 끝까지 다 파고들었으면 다시 원상복귀 해줘야함
        }

    }
    }
}