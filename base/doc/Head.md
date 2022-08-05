# Heap类的实现

### 描述：小顶堆是一棵完全二叉树，且父节点的值小于左右节点  
### 实现数据结构
*由于完全二叉树的特性，所有可以将小顶堆存储在数组中，其中根节点保存在数组的头部，子结点自左向右一次储存*
### 关键函数
- push : 插入函数，将一个节点插入小顶堆上，且经过调整保证小顶堆的特性
- pop : 弹出函数，将根节点弹出，然后调整保证小顶堆的特性
#### push函数实现：
- 第一步：将新的节点直接放置在数组的最后
- 第二步：上滤：将新节点不断与其父节点进行比较，保证父节点大于子节点
#### pop函数实现：
- 第一步：直接取出节点，并将数组头元素与尾元素交换，删除尾元素
- 第二步：下滤：调节当前根节点的次序，将其与子结点中最小的元素互换