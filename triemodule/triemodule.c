#include <Python.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/times.h>

// TODO: Any way to make use of multiple threads?

#define STRINGSBUFSIZE 8000
#define MAXTEXTLEN     8000

typedef struct Node {
  struct Node * childNodes;
  struct Node * siblings;
  bool isWordEnding;
  char character;
} Node;

static Node *startNode;

char * allSubstrings = NULL;
char * nextSubstring;

PyObject * initializeTrie(PyObject *self, PyObject *args);
PyObject * getAllSubstrings ( PyObject *self, PyObject *args );

PyObject * populate(PyObject *self, PyObject *args);

Node *addChildNode(Node *parentNode, char ch, bool endsWord) {
  Node * n;
  n = (Node*) malloc( sizeof(Node) );
  n->childNodes = 0;
  n->character = ch;
  n->isWordEnding = endsWord;
  // Update parent node.
  // Insert node as parent's first child in chain.
  // Insert parent's previous first child as first sibling in chain.

  if (parentNode->childNodes == NULL) {
      parentNode->childNodes = n;
      n->siblings = NULL;
  }
  else {
      Node * tempNode;
      tempNode = parentNode->childNodes;
      parentNode->childNodes = n;
      n->siblings = tempNode;
  }
  return n;
}

PyObject * initializeTrie(PyObject *self, PyObject *args) {

   // Allocate memory for startNode and populate it.
   startNode = (Node*) malloc( sizeof(Node) );
   startNode->childNodes = NULL;
   startNode->siblings = NULL;
   startNode->character = '\0';
   return Py_BuildValue("i", 1);
}

// Determine if input character matches character in the node.
bool match(Node * n, char ch) {

  if (ch == n->character) {
      return true;
  }
  else {
      return false;
  }
}

bool search (char * inStr, Node ** lastNodePtr, int * charPos) {

  /* Strategy: At any node, find the first child. Look through
   * all of its siblings to find a match. If no match, you're
   * done. If a match, make that node current and repeat.
   * LastNode is the last good node visited, even if the
   * full string was not found.
   */

  Node * node;
  int position = 0;
  char ch = inStr[position];
  bool isMatch = false;
  int lastInputIndex = strlen(inStr) - 1;

  *lastNodePtr = startNode;
  if (startNode->childNodes == NULL) {
      return false;
  }
  else {
    node = startNode->childNodes;
  }
  *charPos = 0;

  while (! ((node == NULL) || isMatch)) {
      isMatch = match(node, ch);
      if (isMatch) {   // proceed to next character and set of child nodes
          *lastNodePtr = node;
          if (position == lastInputIndex) {
              return true;
          }
          ++position;           // TODO: combine this and the following statement
          ch = inStr[position];
          node = node->childNodes;
          (*charPos)++;
          isMatch = false;
      }
      else {           // continue to examine siblings
          if (node->siblings != NULL) {
              node = node->siblings;
          }
          else {
              (*charPos)--;
              return ( false );
          }
      }
  }
  (*charPos)--;
  return false;
}

bool insert(char * str) {

  Node * lastNode = NULL;
  int posInString = -1;
  search(str, &lastNode, &posInString);
  if (lastNode == NULL) {
    printf("Error: lastNode is null. Should not have happened.\n");
    return false;
  }
  if (posInString == strlen(str) - 1) {
    lastNode->isWordEnding = true;
  }
  int i;
  Node * newNode;
  bool endsWord = false;
  for (i = posInString + 1; i < strlen(str); i++) {
    if (i == strlen(str) - 1) {
      endsWord = true;
    }
    newNode = addChildNode(lastNode, str[posInString+1], endsWord);
    newNode->character = str[i];
    lastNode = newNode;
  }
  return true;
}


int getSubstrings (char * inStr, Node * node) {

  if (! node) {
    printf("ERROR: getSubstrings: root node is null! Now allowable.\n");
    return 0;
  }

  if (! allSubstrings) {
    allSubstrings = calloc(STRINGSBUFSIZE, sizeof(char));
    nextSubstring = allSubstrings;
  }
  if (node->isWordEnding) {
    strcpy(nextSubstring, inStr); 
    nextSubstring += strlen(inStr) + 1;
  }

  Node * child;
  Node * sib;
  char * tempStr = malloc(2 * sizeof(char));
  char * newStr = malloc(200 * sizeof(char));
  tempStr[1] = '\0';

  if (node->childNodes) {
    child = node->childNodes;
    tempStr[0] = child->character;
    strcpy(newStr, inStr);
    strcat(newStr, tempStr);
    getSubstrings(newStr, child);

    if (child->siblings) {
      for (sib = child->siblings; sib; sib = sib->siblings) {
        tempStr[0] = sib->character;
        strcpy(newStr, inStr);
        strcat(newStr, tempStr);
        getSubstrings(newStr, sib);
      }
    }
  }

  free(tempStr);
  free(newStr);
  return 1;
 
}

PyObject * getAllSubstrings ( PyObject *self, PyObject *args ) {
  char * inStr;
  Node * rootNode = startNode;
  int posInString = 0;

  if (!PyArg_ParseTuple(args, "s",  &inStr)) {
    return Py_BuildValue("[]", 0);
  }

  allSubstrings = NULL;
 
  // Convert inStr to lowercase for case-insensitive comparison
  char * c;
  for (c = inStr; *c; c++) {
    *c = tolower(*c);
  }

  if ( (strcmp(inStr, "")) && ( ! search(inStr, &rootNode, &posInString)) ) {
    printf("\n");
    return Py_BuildValue("[]", 0);
  }

  getSubstrings (inStr, rootNode);

  char * substr = allSubstrings;
  char * text = calloc(MAXTEXTLEN, sizeof(char));

  typedef struct ItemContainer {
    PyObject * item;
    struct ItemContainer * next;
  } ItmCont;

  ItmCont * itemContainer = malloc(sizeof(ItmCont));
  itemContainer->item = NULL;
  itemContainer->next = NULL;
  ItmCont * head = itemContainer;
  int numItems = 0;

  while (*substr != '\0') {
    numItems++;
    if (itemContainer->item != NULL) {
      itemContainer->next = malloc(sizeof(ItmCont));
      itemContainer = itemContainer->next;
      itemContainer->next = NULL;
    }
    itemContainer->item = Py_BuildValue("s",substr);
    Py_INCREF(itemContainer->item);

    sprintf(text + strlen(text), "%s\n", substr);
    substr += (strlen(substr) + 1);
  }

  printf("%s", text);
  free (text);
  free ( allSubstrings );

  PyObject * listOfNames = PyList_New(numItems); 
  int i = 0;
  while (head) {
    PyList_SetItem(listOfNames, i, head->item);
    head = head->next;
    i++;
  }

  return listOfNames;
}


PyObject * populate(PyObject *self, PyObject *args) {

  char * inStr;
  if (!PyArg_ParseTuple(args, "s",  &inStr)) {
    return Py_BuildValue("i", 0);
  }

  // ensure lowercase for case-insensitivity
  char *c;
  for (c = inStr; *c; c++) {
    *c = tolower(*c);
  }

  insert(inStr);
  return Py_BuildValue("s", allSubstrings);

}


static char trie_doc[] =
"This module implements a trie data structure.";

static char trie_init_doc[] =
"Initializes the trie data structure.";

static char trie_populate_doc[] =
"Insert the input string into the trie.";

static char trie_substrings_doc[] = 
"Returns all stored substrings of the entered string.";

static PyMethodDef trie_methods[] = {
    {"init", initializeTrie, METH_VARARGS, trie_init_doc},
    {"populate", populate, METH_VARARGS, trie_populate_doc},
    {"subs", getAllSubstrings, METH_VARARGS, trie_substrings_doc},
    {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC
inittrie(void)
{
  Py_InitModule3("trie", trie_methods, trie_doc);
}

