#ifndef TRIMMER_H
#define TRIMMER_H

void trimAll(TreeNode* top);

void trimFolding(TreeNode* top);

void trimSimpleTailCall(TreeNode* top, int state);
void trimInlineTailCall(TreeNode* top, int state);

#endif

