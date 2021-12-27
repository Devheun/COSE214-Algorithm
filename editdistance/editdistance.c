#include <stdlib.h>
#include <string.h>
#include <stdio.h>
//교체와 매치는 같이 들어가면 X
#define INSERT_OP      0x01 //0001
#define DELETE_OP      0x02 //0010
#define SUBSTITUTE_OP  0x04 //0100
#define MATCH_OP       0x08 //1000
#define TRANSPOSE_OP   0x10 //10000

#define INSERT_COST	1
#define DELETE_COST	1
#define SUBSTITUTE_COST	1
#define TRANSPOSE_COST	1

// 재귀적으로 연산자 행렬을 순회하며, 두 문자열이 최소편집거리를 갖는 모든 가능한 정렬(alignment) 결과를 출력한다.
// op_matrix : 이전 상태의 연산자 정보가 저장된 행렬 (1차원 배열임에 주의!)
// col_size : op_matrix의 열의 크기
// str1 : 문자열 1
// str2 : 문자열 2
// n : 문자열 1의 길이
// m : 문자열 2의 길이
// level : 재귀호출의 레벨 (0, 1, 2, ...)
// align_str : 정렬된 문자쌍들의 정보가 저장된 문자열 배열 예) "a - a", "a - b", "* - b", "ab - ba"
static void backtrace_main( int *op_matrix, int col_size, char *str1, char *str2, int n, int m, int level, char align_str[][8]);

// 강의 자료의 형식대로 op_matrix를 출력 (좌하단(1,1) -> 우상단(n, m))
// 각 연산자를 다음과 같은 기호로 표시한다. 삽입:I, 삭제:D, 교체:S, 일치:M, 전위:T
void print_matrix( int *op_matrix, int col_size, char *str1, char *str2, int n, int m);

// 두 문자열 str1과 str2의 최소편집거리를 계산한다.
// return value : 최소편집거리
// 이 함수 내부에서 print_matrix 함수와 backtrace 함수를 호출함
int min_editdistance( char *str1, char *str2);

////////////////////////////////////////////////////////////////////////////////
// 세 정수 중에서 가장 작은 값을 리턴한다.
static int __GetMin3( int a, int b, int c)
{
	int min = a;
	if (b < min)
		min = b;
	if(c < min)
		min = c;
	return min;
}

////////////////////////////////////////////////////////////////////////////////
// 네 정수 중에서 가장 작은 값을 리턴한다. //transpose까지 고려할때 4개 고려해야함
static int __GetMin4( int a, int b, int c, int d)
{
	int min = __GetMin3( a, b, c);
	return (min > d) ? d : min;
}

////////////////////////////////////////////////////////////////////////////////
// 정렬된 문자쌍들을 출력
void print_alignment( char align_str[][8], int level)
{
	int i;
	
	for (i = level; i >= 0; i--)
	{
		printf( "%s\n", align_str[i]);
	}
}

////////////////////////////////////////////////////////////////////////////////
// backtrace_main을 호출하는 wrapper 함수
// str1 : 문자열 1
// str2 : 문자열 2
// n : 문자열 1의 길이
// m : 문자열 2의 길이
void backtrace( int *op_matrix, int col_size, char *str1, char *str2, int n, int m)
{
	char align_str[n+m][8]; // n+m strings
	
	backtrace_main( op_matrix, col_size, str1, str2, n, m, 0, align_str);
	
}

////////////////////////////////////////////////////////////////////////////////
int main()
{
	char str1[30];
	char str2[30];
	
	int distance;
	
	fprintf( stderr, "INSERT_COST = %d\n", INSERT_COST);
	fprintf( stderr, "DELETE_COST = %d\n", DELETE_COST);
	fprintf( stderr, "SUBSTITUTE_COST = %d\n", SUBSTITUTE_COST);
	fprintf( stderr, "TRANSPOSE_COST = %d\n", TRANSPOSE_COST);
	
	while( fscanf( stdin, "%s\t%s", str1, str2) != EOF)
	{
		printf( "\n==============================\n");
		printf( "%s vs. %s\n", str1, str2);
		printf( "==============================\n");
		
		distance = min_editdistance( str1, str2);
		
		printf( "\nMinEdit(%s, %s) = %d\n", str1, str2, distance);
	}
	return 0;
}

// 두 문자열 str1과 str2의 최소편집거리를 계산한다.
// return value : 최소편집거리
// 이 함수 내부에서 print_matrix 함수와 backtrace 함수를 호출함
int min_editdistance(char* str1, char* str2) {
	
	int n = strlen(str1);
	int m = strlen(str2);

	int i, j;
	int d[n + 1][m + 1]; //d는 거리값, 하나씩 더 더해진 이유는 문자열 길이 나타내려고 (왼쪽과 아래쪽)
	int op_matrix[(n+1)*(m+1)]; //왼쪽부분과 아래쪽 부분도 초기화 해야한다. //왼쪽부분은 삭제연산으로 초기화, 아래쪽 부분은 삽입연산으로 초기화
	memset(d, 0, sizeof(d));
	memset(op_matrix, 0, sizeof(op_matrix));

	for (int i = 0; i <= n;i++) {
		d[i][0] = i;
		if (i > 0) {
			op_matrix[i*(m+1)]= DELETE_OP;
		}
	}

	for (int j = 0; j <= m; j++) {
		d[0][j] = j;
		if (j > 0) {
			op_matrix[j] = INSERT_OP;
		}
	}

	//교체 또는 일치 -> 삽입 -> 삭제 -> 전위 순으로 진행해야함
	
	for (int i = 1; i <= n; i++) {
		for (int j = 1; j <= m; j++) {

			//일치와 교체를 먼저 검사해주어야한다.

			int dist;
			if (str1[i - 1] == str2[j - 1]) {
				dist = 0;
			}
			else dist = SUBSTITUTE_COST;


			if ((i>1) && (j>1) && (str1[i - 2] == str2[j - 1]) && (str1[i - 1] == str2[j - 2])) { //전위가 가능할때
				
					d[i][j] = __GetMin4(d[i - 1][j - 1] + dist, d[i - 1][j] + DELETE_COST, d[i][j - 1] + INSERT_COST,d[i-2][j-2]+TRANSPOSE_COST);

					if (d[i][j] == d[i - 1][j - 1] + dist) { //교체
						if (dist == 0) {
							op_matrix[i * (m + 1) + j] += MATCH_OP;
						}
						else op_matrix[i * (m + 1) + j] += SUBSTITUTE_OP;

					}

					if (d[i][j] == d[i][j - 1] + INSERT_COST) {
						op_matrix[i * (m + 1) + j] += INSERT_OP;
					}
					if (d[i][j] == d[i - 1][j] + DELETE_COST) {
						op_matrix[i * (m + 1) + j] += DELETE_OP;
					}
					if (d[i][j] == d[i - 2][j - 2] + TRANSPOSE_COST) {
						op_matrix[i * (m + 1) + j] += TRANSPOSE_OP;
					}

				

			}

			else { //가능하지 않을때 if로 그림들 다 그려주기
				d[i][j] = __GetMin3(d[i - 1][j - 1] + dist, d[i - 1][j] + DELETE_COST, d[i][j - 1] + INSERT_COST);


				if (d[i][j] == d[i - 1][j - 1] + dist) { //교체
					if (dist == 0) {
						op_matrix[i * (m + 1) + j] += MATCH_OP;
					}
					else op_matrix[i * (m + 1) + j] += SUBSTITUTE_OP;
					
				}

				if (d[i][j] == d[i][j - 1] + INSERT_COST) {
					op_matrix[i * (m + 1) + j] += INSERT_OP;
				}

				if (d[i][j] == d[i - 1][j] + DELETE_COST) {
					op_matrix[i * (m + 1) + j] += DELETE_OP;
				}
			}
		}
	}

	print_matrix(op_matrix, m+1, str1, str2, n, m);
	backtrace(op_matrix, m + 1, str1, str2, n, m);

	return d[n][m];

}

// 강의 자료의 형식대로 op_matrix를 출력 (좌하단(1,1) -> 우상단(n, m))
// 각 연산자를 다음과 같은 기호로 표시한다. 삽입:I, 삭제:D, 교체:S, 일치:M, 전위:T
void print_matrix(int* op_matrix, int col_size, char* str1, char* str2, int n, int m) {

	//&연산자로 체크해주면 될 듯
	//교체 또는 일치 -> 삽입 -> 삭제 -> 전위 순으로 진행해야함

	char letter[5];

	for (int i = n; i >= 1; i--) {
		printf("%c\t", str1[i - 1]);
		for (int j = 1; j <= m; j++) {
			strcpy(letter, "");
			if (SUBSTITUTE_OP & op_matrix[col_size * i + j])strcat(letter, "S");
			if (MATCH_OP & op_matrix[col_size * i + j])strcat(letter, "M");
			if (INSERT_OP & op_matrix[col_size * i + j])strcat(letter, "I");
			if (DELETE_OP & op_matrix[col_size * i + j])strcat(letter, "D");
			if (TRANSPOSE_OP & op_matrix[col_size * i + j])strcat(letter, "T");

			printf("%s\t", letter);
		}
		printf("\n");
	}
	printf("\t");

	for (int i = 1; i <= m; i++) {
		printf("%c\t", str2[i - 1]);
	}
	printf("\n");
	printf("\n");
}

//backtracking을 해야하니 재귀적으로 사용
//op matrix는 연산자 행렬, SD D MD D S같은거 담고 있는것,개념적으로는 2차원행렬
//col size는 1차원행렬가지고 2차원 행렬 표현하려고하니 필요
static void backtrace_main(int* op_matrix, int col_size, char* str1, char* str2, int n, int m, int level, char align_str[][8]) {
	//맨 마지막 level에서 print_alignment 호출해서 출력

	static int count;
	if (level == 0)count = 1;
	
	if ((n == 0) && (m == 0)) {
		printf("[%d] ==============================\n", count);
		count++;
		print_alignment(align_str, level-1); //n==0이고 m==0일땐 필요없으니 level 하나 낮춰줘야함.
		printf("\n");
		return;
	}

	if ((SUBSTITUTE_OP & op_matrix[col_size * n + m]) || (MATCH_OP & op_matrix[col_size * n + m])) {
		strcpy(align_str[level], "x - x"); //x위치 0, 두번째 x위치 4
		align_str[level][0] = str1[n - 1];
		align_str[level][4] = str2[m - 1];
		backtrace_main(op_matrix, col_size, str1, str2, n - 1, m - 1, level + 1, align_str);
	}

	if (DELETE_OP & op_matrix[col_size * n + m]) {
		strcpy(align_str[level], "x - *"); //x위치 0, 두번째 x위치 4
		align_str[level][0] = str1[n - 1];
		backtrace_main(op_matrix, col_size, str1, str2, n - 1, m, level + 1, align_str);
	}

	if (INSERT_OP & op_matrix[col_size*n+m]) {
		strcpy(align_str[level], "* - x"); //x위치 0, 두번째 x위치 4
		align_str[level][4] = str2[m - 1];
		backtrace_main(op_matrix, col_size, str1, str2, n, m - 1, level + 1, align_str);
	}

	if (TRANSPOSE_OP & op_matrix[col_size * n + m]) {
		strcpy(align_str[level], "xy - yx"); //transpose는 총 8칸 필요
		align_str[level][0] = str1[n-2];
		align_str[level][1] = str1[n-1];
		align_str[level][5] = str1[n - 1];
		align_str[level][6] = str1[n - 2];
		backtrace_main(op_matrix, col_size, str1, str2, n-2, m-2, level + 1, align_str);
	}




}
//op[i][j] ==> op[i*col_size+j]