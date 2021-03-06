// stdafx.cpp : Quelldatei, die nur die Standard-Includes einbindet.
// bintreee.pch ist der vorkompilierte Header.
// stdafx.obj enthält die vorkompilierten Typinformationen.

#include "stdafx.h"

// TODO: Auf zusätzliche Header verweisen, die in STDAFX.H
// und nicht in dieser Datei erforderlich sind.

//#include <optional>


/*
template<class Type>
class bintree {

class bintreeElement {
friend class bintree;
std::shared_ptr<bintreeElement> leftEl;
std::shared_ptr<bintreeElement> rightEl;
Type value;
public:
bintreeElement(Type v) : value(v) {}
std::shared_ptr<bintreeElement> addElem(Type v) {
auto me = this;//would be nicer in ASM. Prevents stack overflow
beg:
if (v < me->value) {
if (!me->leftEl) return me->leftEl = std::make_shared<bintreeElement>(v);
me = me->leftEl.get();
goto beg;//set ecx and jmp back
} else {
if (!me->rightEl) return me->rightEl = std::make_shared<bintreeElement>(v);
me = me->rightEl.get();
goto beg;
}
}

void levelCheck(uint32_t level, uint32_t& maxLevel) const {    //#FIXME stackoverflow
if (level > maxLevel) maxLevel = level;
if (leftEl) {
leftEl->levelCheck(level + 1, maxLevel);
}

if (rightEl) {
rightEl->levelCheck(level + 1, maxLevel);
}
}

void inOrder(std::function<void(Type)> func) {    //#FIXME stackoverflow
if (leftEl) {
leftEl->inOrder(func);
}

func(value);

if (rightEl) {
rightEl->inOrder(func);
}
}

void preOrder(std::function<void(Type)> func) {    //#FIXME stackoverflow
func(value);

if (leftEl) {
leftEl->inOrder(func);
}

if (rightEl) {
rightEl->inOrder(func);
}
}

void postOrder(std::function<void(Type)> func) {    //#FIXME stackoverflow
if (leftEl) {
leftEl->inOrder(func);
}

if (rightEl) {
rightEl->inOrder(func);
}
func(value);
}




operator Type() const {
return value;
}
bool hasLeft() const noexcept { return leftEl.get(); }
bool hasRight() const noexcept { return rightEl.get(); }
Type getLeftElement() const {
return *leftEl;
}
Type getRightElement() const {
return *rightEl;
}
uint64_t countElements() const noexcept { ////#FIXME Stack overflow
return
1
+
(leftEl ? leftEl->countElements() : 0)
+
(rightEl ? rightEl->countElements() : 0);
}

bool remove(std::shared_ptr<bintreeElement>& root, std::shared_ptr<bintreeElement>& me, std::shared_ptr<bintreeElement>& parent, Type el) {
if (value != el) {
if (el < value && leftEl) return leftEl->remove(root,leftEl, me, el);
if (el > value && rightEl) return rightEl->remove(root,rightEl, me, el);
return false; //elem doesn't exist
}
if (parent->leftEl == me) {//We are left elem of parent
if (!leftEl && !rightEl) {//No sub elements. Just delete us.
parent->leftEl = nullptr;
return true;
}
if (!leftEl) //One sub element. Just move it up.
return (bool)(parent->leftEl = rightEl);
if (!rightEl) //One sub element. Just move it up.
return (bool)(parent->leftEl = leftEl);

//Two sub elements
auto we = parent->leftEl;
parent->leftEl = rightEl; //move right elem to parent
root->insertElement(leftEl);
return true;
} else {//we are right elem of parent
if (!leftEl && !rightEl) {//No sub elements. Just delete us
parent->rightEl = nullptr;
return true;
}
if (!leftEl) //One sub element. Just move it up.
return (bool)(parent->rightEl = rightEl);
if (!rightEl) //One sub element. Just move it up.
return (bool)(parent->rightEl = leftEl);

//Two sub elements
auto we = parent->rightEl;
parent->rightEl = rightEl; //move right elem to parent
root->insertElement(leftEl);
return true;
}

}

Type minValue() const {
if (leftEl) return leftEl->minValue();
return value;
}
Type maxValue() const {
if (rightEl) return rightEl->minValue();
return value;
}

bool contains(Type val) {
if (value == val) return true;
if (val < value && leftEl) return leftEl->contains(val);
if (val > value && rightEl) return rightEl->contains(val);
return false;
}
void insertElement(std::shared_ptr<bintreeElement> el) {
if (el->value < value && leftEl) return leftEl->insertElement(el);
if (el->value > value && rightEl) return rightEl->insertElement(el);
if (el->value < value && !leftEl) {
leftEl = el;
return;
}
if (el->value > value && !rightEl) {
rightEl = el;
return;
}
}
};

std::shared_ptr<bintreeElement> root;
uint32_t elemCount = 0;
public:

void inOrder(std::function<void(Type)> func) { //O(N)
if (!root) return;
root->inOrder(func);
}

void preOrder(std::function<void(Type)> func) { //O(N)
if (!root) return;
root->preOrder(func);
}

void postOrder(std::function<void(Type)> func) { //O(N)
if (!root) return;
root->postOrder(func);
}

uint32_t levelCheck() {//O(N) visits every element
uint32_t maxLevel = 0;
if (!root) return 0;
root->levelCheck(0, maxLevel);
return maxLevel;
}



Type& insert(Type elem) {//O(N) on empty tree or worst case. O(log2 N) on balanced tree
++elemCount; //#TODO we may reject duplicates
if (!root) {
root = std::make_shared<bintreeElement>(elem);
return root->value;
}
return root->addElem(elem)->value;
}

void balance() {//std::optional<Type> to_remove = {}
std::vector<Type> elements;
inOrder([&elements](Type el) {
elements.emplace_back(el);
});
bool wasSorted = std::is_sorted(elements.begin(), elements.end());
if (!wasSorted) __debugbreak();
//if (to_remove) std::remove_if(elements.begin(), elements.end(), [&to_remove](Type el) {
//    return (el == *to_remove);
//});
//reinsert
root = std::make_shared<bintreeElement>(0);
insSortedVec(root, elements, 0, elements.size() - 1);

}



void remove(Type elem) {//Same as insert O(N) to O(log2 N) to find element. If element has 2 subelements then another insert with O(N) to O(log2 N)
if (!root) return;
if (root->value == elem) root = nullptr;//#FIXME
else {
if (root->remove(root, root, root, elem))
--elemCount;
//if (!root->remove(root, root, root, elem))
//    balance(elem);
}
}




uint64_t countTraverse() const noexcept {//O(N)
if (!root) return 0;
return root->countElements();
}
uint64_t count() const noexcept {//O(1)
return elemCount;
}
Type minValue() const noexcept {//O(1) min. If left branch doesn't exist. O(depth) at max. Only traverses left
if (!root) return 0;
return root->minValue();
}
Type maxValue() const noexcept {//O(1) min. If right branch doesn't exist. O(depth) at max. Only traverses right
if (!root) return 0;
return root->maxValue();
}
bool contains(Type val) {//(log2 N) to O(N) traverses just like insert
if (!root) return false;
return root->contains(val);
}

private:
//Broken
void insSortedVec(std::shared_ptr<bintreeElement> base, std::vector<Type> &elems, int32_t start, int32_t end) {
int32_t midIndex = (start + end) / 2;
if (contains(elems[midIndex])) __debugbreak();
base->value = elems[midIndex];
if (start + 1 <= end) {
if (start < midIndex - 1) {
base->leftEl = std::make_shared<bintreeElement>(0);
insSortedVec(base->leftEl, elems, start, midIndex - 1);
} else {
base->leftEl = std::make_shared<bintreeElement>(0);
insSortedVec(base->leftEl, elems, start, midIndex - 1);
}



if (midIndex + 1 < end) {
base->rightEl = std::make_shared<bintreeElement>(0);
insSortedVec(base->rightEl, elems, midIndex + 1, end);
} else {
base->rightEl = std::make_shared<bintreeElement>(0);
insSortedVec(base->rightEl, elems, midIndex + 1, end);
}

}

}

};
*/