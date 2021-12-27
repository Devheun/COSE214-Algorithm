#include <stdio.h>
#include <stdlib.h>

#define PEASANT 0x08
#define WOLF	0x04
#define GOAT	0x02
#define CABBAGE	0x01

// 주어진 상태 state의 이름(마지막 4비트)을 화면에 출력
// 예) state가 7(0111)일 때, "<0111>"을 출력
static void print_statename(FILE* fp, int state);

// 주어진 상태 state에서 농부, 늑대, 염소, 양배추의 상태를 각각 추출하여 p, w, g, c에 저장
// 예) state가 7(0111)일 때, p = 0, w = 1, g = 1, c = 1
static void get_pwgc(int state, int* p, int* w, int* g, int* c) {
	*p = state / 8;
	*w = (state % 8) / 4;
	*g = (state % 4) / 2;
	*c = state % 2;
	return;
}

// 허용되지 않는 상태인지 검사
// 예) 농부없이 늑대와 염소가 같이 있는 경우 / 농부없이 염소와 양배추가 같이 있는 경우
// return value: 1 허용되지 않는 상태인 경우, 0 허용되는 상태인 경우
static int is_dead_end(int state);

// state1 상태에서 state2 상태로의 전이 가능성 점검
// 농부 또는 농부와 다른 하나의 아이템이 강 반대편으로 이동할 수 있는 상태만 허용
// 허용되지 않는 상태(dead-end)로의 전이인지 검사
// return value: 1 전이 가능한 경우, 0 전이 불이가능한 경우 
static int is_possible_transition(int state1, int state2);

// 상태 변경: 농부 이동
// return value : 새로운 상태
static int changeP(int state);

// 상태 변경: 농부, 늑대 이동
// return value : 새로운 상태, 상태 변경이 불가능한 경우: -1
static int changePW(int state);

// 상태 변경: 농부, 염소 이동
// return value : 새로운 상태, 상태 변경이 불가능한 경우: -1
static int changePG(int state);

// 상태 변경: 농부, 양배추 이동
// return value : 새로운 상태, 상태 변경이 불가능한 경우: -1 
static int changePC(int state);

// 주어진 state가 이미 방문한 상태인지 검사
// return value : 1 visited, 0 not visited
static int is_visited(int visited[], int level, int state); //level까지만 방문 체크 해주면 됨

// 방문한 상태들을 차례로 화면에 출력
static void print_states(int visited[], int count); //count는 위의 함수 level과 같은 용도

// recursive function
static void dfs_main(int state, int goal_state, int level, int visited[]);

////////////////////////////////////////////////////////////////////////////////
// 상태들의 인접 행렬을 구하여 graph에 저장
// 상태간 전이 가능성 점검
// 허용되지 않는 상태인지 점검 
void make_adjacency_matrix(int graph[][16]);

// 인접행렬로 표현된 graph를 화면에 출력
void print_graph(int graph[][16], int num); //num은 16 주면 됨

// 주어진 그래프(graph)를 .net 파일로 저장
// pgwc.net 참조
void save_graph(char* filename, int graph[][16], int num);

////////////////////////////////////////////////////////////////////////////////
// 깊이 우선 탐색 (초기 상태 -> 목적 상태)
void depth_first_search(int init_state, int goal_state)
{
	int level = 0;
	int visited[16] = { 0, }; // 방문한 정점을 저장

	dfs_main(init_state, goal_state, level, visited); //초기상태는 visited[0]에 들어감, 다음상태는 visited[1]=3 이런 식으로
}

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
	int graph[16][16] = { 0, };
	// 인접 행렬 만들기 (전이 가능성에 따라 0 또는 1로)
	make_adjacency_matrix(graph);

	// 인접 행렬 출력 (only for debugging) 제출할때는 print_graph 주석 처리해서
	print_graph(graph, 16);

	// .net 파일 만들기
	save_graph( "pwgc.net", graph, 16);

	// 깊이 우선 탐색
	depth_first_search( 0, 15); // initial state, goal state

	return 0;
}

// 상태 변경: 농부 이동
// return value : 새로운 상태
static int changeP(int state) {
	if (state <= 7)return state + PEASANT;
	else return state - PEASANT;
}

// 상태 변경: 농부, 늑대 이동
// return value : 새로운 상태, 상태 변경이 불가능한 경우: -1
static int changePW(int state) {
	if (state >= 0 && state <= 3) {
		return state + PEASANT + WOLF;
	}
	else if (state >= 12 && state <= 15) {
		return state - PEASANT - WOLF;
	}
	else return -1;
}

// 상태 변경: 농부, 염소 이동
// return value : 새로운 상태, 상태 변경이 불가능한 경우: -1
static int changePG(int state) {
	if (state == 0 || state == 1 || state == 4 || state == 5)return state + PEASANT + GOAT;
	else if (state == 10 || state == 11 || state == 14 || state == 15)return state - PEASANT - GOAT;
	else return -1;
}

// 상태 변경: 농부, 양배추 이동
// return value : 새로운 상태, 상태 변경이 불가능한 경우: -1 
static int changePC(int state) {
	if (state == 0 || state == 2 || state == 4 || state == 6)return state + PEASANT + CABBAGE;
	else if (state == 9 || state == 11 || state == 13 || state == 15)return state - PEASANT - CABBAGE;
	else return -1;
}

// 주어진 state가 이미 방문한 상태인지 검사
// return value : 1 visited, 0 not visited
static int is_visited(int visited[], int level, int state) { //level까지만 방문 체크 해주면 됨
	
	for (int i = 0; i <= level; i++) {
		if (visited[i] == state) {
			return 1;
		}
	}
	return 0;
}


// 허용되지 않는 상태인지 검사
// 예) 농부없이 늑대와 염소가 같이 있는 경우 / 농부없이 염소와 양배추가 같이 있는 경우
// return value: 1 허용되지 않는 상태인 경우, 0 허용되는 상태인 경우
static int is_dead_end(int state) {

	if (state == GOAT + WOLF || state==GOAT + WOLF + CABBAGE || state==GOAT + CABBAGE || state==PEASANT || state==PEASANT + CABBAGE || state==PEASANT + WOLF)return 1;
	else return 0;
}

// state1 상태에서 state2 상태로의 전이 가능성 점검
// 농부 또는 농부와 다른 하나의 아이템이 강 반대편으로 이동할 수 있는 상태만 허용
// 허용되지 않는 상태(dead-end)로의 전이인지 검사
// return value: 1 전이 가능한 경우, 0 전이 불이가능한 경우 
static int is_possible_transition(int state1, int state2) {
	int transition = state1 ^ state2;

	if (is_dead_end(state1) || is_dead_end(state2))return 0; //두 state들이 각각 가능한지부터 체크

	if (transition == PEASANT + GOAT || transition==PEASANT || transition==PEASANT+WOLF || transition==PEASANT+CABBAGE)return 1;

	return 0;
}

// 상태들의 인접 행렬을 구하여 graph에 저장
// 상태간 전이 가능성 점검
// 허용되지 않는 상태인지 점검 
void make_adjacency_matrix(int graph[][16]) {

	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 16; j++) {
			if (i == j)continue;
			if (is_possible_transition(i, j)) {
				graph[i][j] = 1;
			}
			else graph[i][j] = 0;
		}
	}
}
// 인접행렬로 표현된 graph를 화면에 출력
void print_graph(int graph[][16], int num) { //num은 16 주면 됨

	for (int i = 0; i < num; i++) {
		for (int j = 0; j < num; j++) {
			fprintf(stderr, "%d\t", graph[i][j]);
		}
		fprintf(stderr, "\n");
	}
}

// 주어진 상태 state의 이름(마지막 4비트)을 화면에 출력
// 예) state가 7(0111)일 때, "<0111>"을 출력
static void print_statename(FILE* fp, int state) {
	fprintf(fp,"<%d%d%d%d>",state/8,(state%8)/4,(state%4)/2,state%2);
}

// 주어진 그래프(graph)를 .net 파일로 저장
// pgwc.net 참조
void save_graph(char* filename, int graph[][16], int num) {

	FILE* fp = fopen(filename, "w");
	fprintf(fp,"*Vertices %d\n", num);
	for (int i = 1; i <= num; i++) {
		fprintf(fp,"%d \"",i);
		print_statename(fp, i-1);
		fprintf(fp,"\"\n");
	}

	fprintf(fp, "*Edges\n");
	for (int i = 1; i <= num; i++) {
		for (int j = i + 1; j <= num; j++) {
			if (graph[i-1][j-1]) {
				fprintf(fp, " %d %d\n", i, j);
			}
		}
	}
	fclose(fp);
}

// recursive function
static void dfs_main(int state, int goal_state, int level, int visited[]) {

	visited[level] = state; //방문 체크

	int p, w, g, c;
	fprintf(stderr, "cur state is <");
	get_pwgc(state, &p, &w, &g, &c);
	fprintf(stderr, "%d%d%d%d> (level %d)\n", p, w, g, c, level);

	if (state == goal_state) {
		fprintf(stderr, "Goal-state found!\n");
		print_states(visited,level);
		return;
	}

	//농부만 움직일때
	int x1 = changeP(state);
	int x2 = is_dead_end(x1);
	int x3 = is_visited(visited,level,x1);
	if (x2) {
		fprintf(stderr, "\tnext state <");
		get_pwgc(x1, &p, &w, &g, &c);
		fprintf(stderr, "%d%d%d%d> is dead-end\n", p, w, g, c);
	}
	else if (x3) {
		fprintf(stderr, "\tnext state <");
		get_pwgc(x1, &p, &w, &g, &c);
		fprintf(stderr, "%d%d%d%d> has been visited\n", p, w, g, c);
	}
	else {
		dfs_main(x1, goal_state, level + 1, visited);
		get_pwgc(state, &p, &w, &g, &c);
		fprintf(stderr, "back to <%d%d%d%d> (level %d)\n", p, w, g, c,level);
	}

	//농부, 늑대 움직일 때
	x1 = changePW(state);
	if (x1 != -1) {
		x2 = is_dead_end(x1);
		x3 = is_visited(visited,level,x1);

		if (x2) {
			fprintf(stderr, "\tnext state <");
			get_pwgc(x1, &p, &w, &g, &c);
			fprintf(stderr, "%d%d%d%d> is dead-end\n", p, w, g, c);
		}
		else if (x3) {
			fprintf(stderr, "\tnext state <");
			get_pwgc(x1, &p, &w, &g, &c);
			fprintf(stderr, "%d%d%d%d> has been visited\n", p, w, g, c);
		}
		else {
			dfs_main(x1, goal_state, level + 1, visited);
			get_pwgc(state, &p, &w, &g, &c);
			fprintf(stderr, "back to <%d%d%d%d> (level %d)\n", p, w, g, c,level);
		}
	}

	//농부, 염소 움직일 때
	x1 = changePG(state);
	if (x1 != -1) {
		x2 = is_dead_end(x1);
		x3 = is_visited(visited,level,x1);

		if (x2) {
			fprintf(stderr, "\tnext state <");
			get_pwgc(x1, &p, &w, &g, &c);
			fprintf(stderr, "%d%d%d%d> is dead-end\n", p, w, g, c);
		}
		else if (x3) {
			fprintf(stderr, "\tnext state <");
			get_pwgc(x1, &p, &w, &g, &c);
			fprintf(stderr, "%d%d%d%d> has been visited\n", p, w, g, c);
		}
		else {
			dfs_main(x1, goal_state, level + 1, visited);
			get_pwgc(state, &p, &w, &g, &c);
			fprintf(stderr, "back to <%d%d%d%d> (level %d)\n", p, w, g, c,level);
		}
	}

	//농부, 양배추 움직일때
	x1 = changePC(state);
	if (x1 != -1) {
		x2 = is_dead_end(x1);
		x3 = is_visited(visited,level,x1);

		if (x2) {
			fprintf(stderr, "\tnext state <");
			get_pwgc(x1, &p, &w, &g, &c);
			fprintf(stderr, "%d%d%d%d> is dead-end\n", p, w, g, c);
		}
		else if (x3) {
			fprintf(stderr, "\tnext state <");
			get_pwgc(x1, &p, &w, &g, &c);
			fprintf(stderr, "%d%d%d%d> has been visited\n", p, w, g, c);
		}
		else {
			dfs_main(x1, goal_state, level + 1, visited);
			get_pwgc(state, &p, &w, &g, &c);
			fprintf(stderr, "back to <%d%d%d%d> (level %d)\n", p, w, g, c,level);
		}
	}
	return;
}
// 방문한 상태들을 차례로 화면에 출력
static void print_states(int visited[], int count) { //count는 위의 함수 level과 같은 용도

	int p, w, g, c;

	for (int i = 0; i <= count; i++) {
		get_pwgc(visited[i], &p, &w, &g, &c);
		fprintf(stderr,"<%d%d%d%d>\n", p, w, g, c);
	}
	fprintf(stderr, "\n");
}
