#include "TimetableRow.h"

// TimetableRow implementation
TimetableRow::TimetableRow()
{
	view[0].setTarget(&slots);
}

void TimetableRow::SetLabel(string newLabel)
{
	label = newLabel;
}