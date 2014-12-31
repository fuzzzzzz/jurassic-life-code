
#ifndef BLURCOMBOBOX_H
#define BLURCOMBOBOX_H

#include "ComboBox.h"

namespace vgui
{

	class BlurComboBox : public ComboBox
	{
		DECLARE_CLASS_SIMPLE( ComboBox, TextEntry );

	public:
		BlurComboBox(Panel *parent, const char *panelName);
		~BlurComboBox();

		MESSAGE_FUNC_INT( ActivateItem, "ActivateItem", itemID );
		//MESSAGE_FUNC( OnMenuItemSelectedBis, "MenuItemSelected" );
	};

}

#endif