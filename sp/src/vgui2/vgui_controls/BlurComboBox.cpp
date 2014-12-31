
#include "../public/vgui_controls/BlurComboBox.h"

using namespace vgui;

DECLARE_BUILD_FACTORY( BlurComboBox );

BlurComboBox::BlurComboBox(Panel *parent, const char *panelName)
: ComboBox(parent, panelName,3,false)
{
	AddItem("Low",NULL);
	AddItem("Medium",NULL);
	AddItem("High",NULL);
	ActivateItemByRow(1);
}

BlurComboBox::~BlurComboBox()
{
}

void BlurComboBox::ActivateItem(int itemID)
{
	DevMsg("Change BlurComboBox item %d\n",itemID);
}

/*void BlurComboBox::OnMenuItemSelectedBis()
{
	DevMsg("Select BlurComboBox item %d\n");
}*/