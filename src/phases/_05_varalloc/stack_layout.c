#include <phases/_05_varalloc/stack_layout.h>
#include <limits.h>
#define POINTER_SIZE (4)

StackLayout *newStackLayout() {
    StackLayout *self = new(StackLayout);

    self->argumentAreaSize = INT_MIN;
    self->localVarAreaSize = INT_MIN;
    self->outgoingAreaSize = INT_MIN;

    return self;
}

// bool isLeafProcedure(StackLayout *stackLayout) {
//     if (stackLayout->outgoingAreaSize == INT_MIN) return false;
//
//     //TODO (assignment 5): Implement this function properly
//     return false;
// }
bool isLeafProcedure(StackLayout *stackLayout) {
	return stackLayout->outgoingAreaSize == -1;
}

// int getFrameSize(StackLayout *stackLayout) {
//     if (stackLayout->localVarAreaSize == INT_MIN || stackLayout->outgoingAreaSize == INT_MIN) return INT_MIN;
//
//     //TODO (assignment 5): Implement this function properly
//     return INT_MIN;
// }
int getFrameSize(StackLayout *stackLayout) {
	int return_old_size = isLeafProcedure(stackLayout)? 0 : POINTER_SIZE;
	int fp_old_size = POINTER_SIZE;
	int outgoingAreaSize = stackLayout->outgoingAreaSize != -1 ? stackLayout->outgoingAreaSize: 0;

	return stackLayout->localVarAreaSize +
		outgoingAreaSize +
		fp_old_size +
		return_old_size;
}

// int getOldFramePointerOffSet(StackLayout *stackLayout) {
//     if (stackLayout->outgoingAreaSize == INT_MIN) return INT_MIN;
//
//     //TODO (assignment 5): Implement this function properly
//     return INT_MIN;
// }
int getOldFramePointerOffSet(StackLayout *stackLayout) {
	//TODO
	int return_old_size = isLeafProcedure(stackLayout)? 0 : POINTER_SIZE;
	int outgoingAreaSize = stackLayout->outgoingAreaSize != -1 ? stackLayout->outgoingAreaSize: 0;
	return outgoingAreaSize + return_old_size;
}

// int getOldReturnAddressOffset(StackLayout *stackLayout) {
//     if (stackLayout->localVarAreaSize == INT_MIN) return INT_MIN;
//
//     //TODO (assignment 5): Implement this function properly
//     return INT_MIN;
// }

int getOldReturnAddressOffset(StackLayout *stackLayout) {
	int fp_old_size = POINTER_SIZE;
	int return_old_size = isLeafProcedure(stackLayout)? 0 : POINTER_SIZE;
	int offset =  stackLayout->localVarAreaSize + fp_old_size + return_old_size;
	return -offset;
}
