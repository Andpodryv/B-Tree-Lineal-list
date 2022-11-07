#include <stdio.h>
#include <Windows.h>
#include <time.h>

//////////////////////////////////////////////////////////////////////   B-TREE     ////////////////////////////////////////////////////////////////////////////////
#define M 5 //2nd degree tree

struct node {
	int n; // the number of elements in the tree is less than M
	int keys[M - 1]; //node elements
	struct node* p[M]; //n+1 children's
}*root = NULL;

enum KeyStatus { Duplicate, SearchFailure, Success, InsertIt, LessKeys }; //different states

enum KeyStatus ins(struct node* ptr, int key, int* upKey, struct node** newnode)
{
	struct node* newPtr, * lastPtr;
	int pos, i, n, splitPos;
	int newKey, lastKey;
	enum KeyStatus value;
	if (!ptr) //if there is no node, then insert
	{
		*newnode = NULL;
		*upKey = key;
		return InsertIt;
	}
	n = ptr->n;
	pos = searchPos(key, ptr->keys, n); //looking for a position at the root
	if (pos < n && key == ptr->keys[pos]) return Duplicate; //if you have not reached the end, and the number you have reached is equal to the key, then Duplicate
	value = ins(ptr->p[pos], key, &newKey, &newPtr);
	if (value != InsertIt) return value;
	//If the number of keys is less than M-1
	if (n < M - 1)
	{
		pos = searchPos(newKey, ptr->keys, n);
		//Shifting keys and pointers to children to insert a new key
		for (i = n; i > pos; i--)
		{
			ptr->keys[i] = ptr->keys[i - 1];
			ptr->p[i + 1] = ptr->p[i];
		}
		//insert the key
		ptr->keys[pos] = newKey;
		ptr->p[pos + 1] = newPtr;
		ptr->n++;
		return Success;
	}
	//If the keys in the nodes are the maximum, and the position of the inserted node is the last
	if (pos == M - 1)
	{
		lastKey = newKey;
		lastPtr = newPtr;
	}
	else //If the keys in the node are maximum and the position of the inserted node is not the last
	{
		lastKey = ptr->keys[M - 2];
		lastPtr = ptr->p[M - 1];
		for (i = M - 2; i > pos; i--)
		{
			ptr->keys[i] = ptr->keys[i - 1];
			ptr->p[i + 1] = ptr->p[i];
		}
		ptr->keys[pos] = newKey;
		ptr->p[pos + 1] = newPtr;
	}
	splitPos = (M - 1) / 2;
	*upKey = ptr->keys[splitPos];

	*newnode = malloc(sizeof(struct node));//right node after separation
	ptr->n = splitPos; //Number of keys for the left subtree
	(*newnode)->n = M - 1 - splitPos;//Number of keys for the right subtree
	for (i = 0; i < (*newnode)->n; i++)
	{
		(*newnode)->p[i] = ptr->p[i + splitPos + 1];
		if (i < (*newnode)->n - 1) (*newnode)->keys[i] = ptr->keys[i + splitPos + 1];
		else (*newnode)->keys[i] = lastKey;
	}
	(*newnode)->p[(*newnode)->n] = lastPtr;
	return InsertIt;
}

void insert(int key)
{
	struct node* newnode;
	int upKey;
	enum KeyStatus value;
	value = ins(root, key, &upKey, &newnode);
	if (value == Duplicate) printf("This key already exists.\n");
	else if (value == InsertIt) //if it is possible to insert into an empty node
	{
		struct node* uproot = root;
		root = (struct node*)malloc(sizeof(struct node));
		root->n = 1;
		root->keys[0] = upKey;
		root->p[0] = uproot;
		root->p[1] = newnode;
	}
}

int searchPos(int key, int* key_arr, int n) //searches for a position in a node
{
	int pos = 0;
	while (pos < n && key > key_arr[pos]) pos++;
	return pos;
}

void display(struct node* ptr, int blanks)
{
	if (ptr)
	{
		int i;
		for (i = 1; i <= blanks; i++) putchar(' ');
		for (i = 0; i < ptr->n; i++) printf("%d ", ptr->keys[i]);
		putchar('\n');
		for (i = 0; i <= ptr->n; i++) display(ptr->p[i], blanks + 10);
	}
}

int search(int key)
{
	int pos, n;
	struct node* ptr = root;
	while (ptr)
	{
		n = ptr->n;
		pos = searchPos(key, ptr->keys, n);
		if (pos < n && key == ptr->keys[pos]) return 1;//if the position has not gone beyond and the key is found, we exit
		ptr = ptr->p[pos];//otherwise we take the child
	}
	return 0; //key not found
}

enum KeyStatus del(struct node* ptr, int key)
{
	int pos, i, pivot, n, min;
	int* key_arr;
	enum KeyStatus value;
	struct node** p, * lptr, * rptr;

	if (!ptr) return SearchFailure; //if the tree is empty
	//Looking for the node value
	n = ptr->n;
	key_arr = ptr->keys;
	p = ptr->p;
	min = (M - 1) / 2;//minimum number of keys

	pos = searchPos(key, key_arr, n);
	if (p[0] == NULL) //if it is a leaf of a tree
	{
		if (pos == n || key < key_arr[pos]) return SearchFailure;//if you have reached the end or there is no key, then the key is not found
		//Shift keys and links to the left
		for (i = pos + 1; i < n; i++)
		{
			key_arr[i - 1] = key_arr[i];
			p[i] = p[i + 1];
		}
		if (ptr == root) min = 1;//there can be (M-1)/2 -1 numbers in the root of the tree
		if (--ptr->n >= min) return Success; //if min keys remain in the tree, taking into account the deletion of the key, then we have deleted the element
		else return LessKeys; //otherwise, there will be few keys left in the node of the tree, and this cannot be

	}

	if (pos < n && key == key_arr[pos])
	{
		struct node* qp = p[pos], * qp1;
		int nkey;
		while (1)//looking for a node in front of the sheet
		{
			nkey = qp->n;
			qp1 = qp->p[nkey];
			if (qp1 == NULL) break;
			qp = qp1;
		}
		key_arr[pos] = qp->keys[nkey - 1];
		qp->keys[nkey - 1] = key;
	}
	value = del(p[pos], key);
	if (value != LessKeys) return value;

	if (pos > 0 && p[pos - 1]->n > min)
	{
		pivot = pos - 1; //rotation for left and right nodes
		lptr = p[pivot];
		rptr = p[pos];
		//value for the right node
		rptr->p[rptr->n + 1] = rptr->p[rptr->n];
		for (i = rptr->n; i > 0; i--)
		{
			rptr->keys[i] = rptr->keys[i - 1];
			rptr->p[i] = rptr->p[i - 1];
		}
		rptr->n++;
		rptr->keys[0] = key_arr[pivot];
		rptr->p[0] = lptr->p[lptr->n];
		key_arr[pivot] = lptr->keys[--lptr->n];
		return Success;
	}
	if ((*p)->n > min)
	{
		pivot = pos; //rotation for left and right nodes
		lptr = p[pivot];
		rptr = p[pivot + 1];
		//Value for the left node
		lptr->keys[lptr->n] = key_arr[pivot];
		lptr->p[lptr->n + 1] = rptr->p[0];
		key_arr[pivot] = rptr->keys[0];
		lptr->n++;
		rptr->n--;
		for (i = 0; i < rptr->n; i++)
		{
			rptr->keys[i] = rptr->keys[i + 1];
			rptr->p[i] = rptr->p[i + 1];
		}
		rptr->p[rptr->n] = rptr->p[rptr->n + 1];
		return Success;
	}

	if (pos == n) pivot = pos - 1;
	else pivot = pos;

	lptr = p[pivot];
	rptr = p[pivot + 1];
	//combine the right and left nodes
	lptr->keys[lptr->n] = key_arr[pivot];
	lptr->p[lptr->n + 1] = rptr->p[0];
	for (i = 0; i < rptr->n; i++)
	{
		lptr->keys[lptr->n + 1 + i] = rptr->keys[i];
		lptr->p[lptr->n + 2 + i] = rptr->p[i + 1];
	}
	lptr->n = lptr->n + rptr->n + 1;
	free(rptr); //deleting the right node
	for (i = pos + 1; i < n; i++)
	{
		key_arr[i - 1] = key_arr[i];
		p[i] = p[i + 1];
	}
	if (ptr == root) min = 1;
	if (--ptr->n >= min) return Success;
	else return LessKeys;

}

void DelNode(int key)
{
	struct node* uproot;
	enum KeyStatus value;
	value = del(root, key);
	switch (value)
	{
	case SearchFailure:
		printf("Key %d not detected.\n", key);
		break;
	case LessKeys:
		uproot = root;
		root = root->p[0];
		free(uproot);
		break;
	}
}

//////////////////////////////////////////////////////////////////   LINEAR LIST   /////////////////////////////////////////////////////////////////////////////////////////////
struct Node
{
	int numb;
	struct Node* next;
};
typedef struct Node* PNode;
PNode Head = NULL;

//Creating a node
PNode CreateNode(int ch)
{
	PNode NewNode = (PNode)malloc(sizeof(struct Node)); // pointer to a new node
	NewNode->numb = ch;
	NewNode->next = NULL; // there is no next node
	return NewNode; // the result of the function is the node address
}

//Adding to the top of the list
PNode AddFirst(PNode NewNode)
{
	NewNode->next = Head;
	Head = NewNode;
	return Head;
}

PNode AddAfter(PNode p, PNode NewNode)
{
	NewNode->next = p->next;
	p->next = NewNode;
	return NewNode;
}

PNode Find(int ch)
{
	PNode q = Head;
	while (q && ch != q->numb) q = q->next;
	return q;
}

PNode SearchNode(int ch) { //to delete
	PNode q = Head;
	PNode p = Head->next;

	while (p && p->numb != ch)
	{
		q = p;
		p = p->next;
	}
	if (!p) return NULL;
	else return q;
}

void DeleteNode(int ch, PNode qp)
{
	PNode q = Head;
	PNode p = qp->next;
	if (Head->numb == ch)
	{
		Head = Head->next;
		free(q);
	}

	else
	{
		qp->next = p->next;
		free(p);
	}
}

int AddBefore(PNode NewNode, int ch)
{
	PNode q = Head;
	PNode p = Head;
	while (p && p->numb != ch) p = p->next;
	if (!p) return 0;
	if (q == p) {
		AddFirst(NewNode); // insert before the first node
		return 1;
	}
	while (q && q->next != p) q = q->next;// looking for a node followed by p
	if (q)
	{
		AddAfter(q, NewNode);
		return 1;
	}//if you find such a node, add a new one after it
}

void DisplayLine()
{
	PNode q = Head;
	printf("Linear list: ");
	while (q)
	{
		printf("%d ", q->numb);
		q = q->next;
	}
	printf("\n\n\n\n\n\n");
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

long perf_time() {
	LARGE_INTEGER time;
	QueryPerformanceCounter(&time);
	return time.QuadPart;
}






int main()
{
	long p;
	int l;
	int razmer;
	int ch;
	int i;
	PNode last = NULL;
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);

	for (int i = 0; i < 10; i++)
	{
		if (i == 0) {
			last = AddFirst(CreateNode(i));
		}
		else  last = AddAfter(last, CreateNode(i));
		insert(i);
	}
	ch = 0;
	putchar('\n');

	//Μενώ
	while (ch != 3)
	{
		int k, new, my, yoy;
		printf("Choose a structure:\n");
		printf("1.Linear list\n");
		printf("2.B-tree \n");
		printf("3.Exit.\n");
		scanf_s("%d", &ch);
		putchar('\n');
		switch (ch)
		{
		case 1:
			printf("1.Search.\n");
			printf("2.Removal.\n");
			printf("3.Insert.\n");
			printf("4.Output a list.\n");
			scanf_s("%d", &k);
			switch (k)
			{
			case 1:
				printf("Enter the number to be found: ");
				scanf_s("%d", &new);
				p = perf_time();
				if (Find(new))
				{
					p = perf_time() - p;
					printf("Number %d found. Search time: %ld\n", new, p);
				}
				else
				{
					p = perf_time() - p;
					printf("This number is not in the list. Search time: %ld\n", p);
				}
				break;


			case 2:
				printf("Enter the number you want to delete: ");
				scanf_s("%d", &new);
				p = perf_time();
				last = SearchNode(new);
				if (last)
				{
					p = perf_time();
					DeleteNode(new, last);
					p = perf_time() - p;
					printf("The number %d has been deleted. Time of deletion: %ld\n", new, p);
				}
				else
				{
					p = perf_time() - p;
					printf("The number %d is missing. Time of deletion:%ld\n", new, p);
				}
				break;

			case 3:
				printf("\enter the number to insert: ");
				scanf_s("%d", &new);
				printf("Select the function:\n");
				printf("1.Paste to the beginning.\n");
				printf("2.Insert before the specified number.\n");
				printf("3.Insert after the specified number.\n");
				scanf_s("%d", &my);

				switch (my)
				{
				case 1:
					last = CreateNode(new);
					p = perf_time();
					AddFirst(last);
					p = perf_time() - p;
					printf("The number is inserted. Time: %ld\n", p);
					break;

				case 2:
					printf("Enter the number before which you want to insert: ");
					scanf_s("%d", &yoy);
					last = CreateNode(new);
					p = perf_time();
					i = AddBefore(last, yoy);
					p = perf_time() - p;
					if (i)
					{
						printf("The number is inserted. Time: %ld\n", p);
						break;
					}
					else printf("Couldn't insert a number.\n");
					break;

				case 3:
					printf("Enter the number after which you want to insert: ");
					scanf_s("%d", &yoy);
					if (Find(yoy))
					{
						PNode lo = Find(yoy);
						last = CreateNode(new);
						p = perf_time();
						AddAfter(lo, last);
						p = perf_time() - p;
						printf("The number is inserted. Time: %ld\n", p);
						break;
					}
					else printf("Couldn't insert a number.\n");
					break;
				}
				break;

			case 4: DisplayLine();
			}
			break;

			//Tree
		case 2:
			printf("1.Insert \n");
			printf("2.Deleting \n");
			printf("3.Search\n");
			printf("4.Display \n");
			scanf_s("%d", &k);
			switch (k)
			{
			case 1:
				printf("Enter the number you want to insert: ");
				scanf_s("%d", &my);
				p = perf_time();
				insert(my);
				p = perf_time() - p;
				printf("Time: %ld\n", p);
				break;
			case 2:
				printf("Enter the number you want to delete: ");
				scanf_s("%d", &my);
				p = perf_time();
				DelNode(my);
				p = perf_time() - p;
				printf("Βπεμÿ: %ld\n", p);
				break;
			case 3:
				printf("Enter the number you want to find: ");
				scanf_s("%d", &my);
				p = perf_time();
				l = search(my);
				p = perf_time() - p;
				if (l) printf("Key %d found. Time: %ld\n", my, p);
				else printf("The %d key was not found. Time: %ld\n", my, p);
				break;
			case 4:
				printf("Tree :\n");
				display(root, 0);
				break;
			}
			printf("\n\n\n\n");
			break;

		case 3: exit(0);
		}
	}
}
