/*
	This file is a part of Awesomium, a library that makes it easy for 
	developers to embed web-content in their applications.

	Copyright (C) 2009 Khrona. All rights reserved. Awesomium is a trademark of Khrona.
*/

#ifndef __WEBKEYBOARDEVENT_H__
#define __WEBKEYBOARDEVENT_H__

#if defined(__APPLE__)
#ifdef __OBJC__
@class NSEvent;
#else
class NSEvent;
#endif
#endif

namespace Awesomium {

#if defined(__WIN32__) || defined(_WIN32)

#include <windows.h>

typedef wchar_t WebUChar;

#elif defined(__APPLE__)

typedef unsigned short WebUChar;

#endif

/**
* A generic keyboard event that can be created from a platform-specific event or
* synthesized from a virtual event. Used by WebView::injectKeyboardEvent.
*/
class _OSMExport WebKeyboardEvent
{
public:
	/**
	* Creates an empty WebKeyboardEvent, you will need to initialize its members
	* yourself. This is most useful for synthesizing your own keyboard events.
	*/
	WebKeyboardEvent();

#if defined(__WIN32__) || defined(_WIN32)
	/**
	* Creates a WebKeyboardEvent directly from a Windows keyboard event message.
	*
	* @note	Valid message types include:
	*		- WM_KEYDOWN
	*		- WM_KEYUP
	*		- WM_SYSKEYDOWN
	*		- WM_SYSKEYUP
	*		- WM_CHAR
	*		- WM_IMECHAR
	*		- WM_SYSCHAR
	*/
	WebKeyboardEvent(UINT msg, WPARAM wparam, LPARAM lparam);
#elif defined(__APPLE__)
	/**
	* Creates a WebKeyboardEvent directly from a Mac OSX NSEvent.
	*/
	WebKeyboardEvent(NSEvent* event);
#endif

	/// An enumeration of the different WebKeyboardEvent types.
	enum Type
	{
		TYPE_KEY_DOWN,	/// Key-Down type
		TYPE_KEY_UP,	/// Key-Up type
		TYPE_CHAR		/// Character input type
	};

	/// An enumeration of the different keyboard modifiers.
	enum Modifiers
	{
		MOD_SHIFT_KEY		= 1 << 0, /// Whether or not a Shift key is down
		MOD_CONTROL_KEY		= 1 << 1, /// Whether or not a Control key is down
		MOD_ALT_KEY			= 1 << 2, /// Whether or not an ALT key is down
		MOD_META_KEY		= 1 << 3, /// Whether or not a meta key (Command-key on Mac, Windows-key on Windows) is down
		MOD_IS_KEYPAD		= 1 << 4, /// Whether or not the key pressed is on the keypad
		MOD_IS_AUTOREPEAT	= 1 << 5, /// Whether or not the character input is the result of an auto-repeat timer.
	};

	/// The type of this WebKeyboardEvent
	Type type;

	/// The current state of the keyboard. Modifiers may be OR'd together to represent multiple values.
	int modifiers;

	/**
	* The virtual key-code associated with this keyboard event. This is either directly
	* from the event (ie, WPARAM on Windows) or via a mapping function. You can see a full 
	* list of the possible virtual key-codes in KeyboardCodes.h
	*/
	int virtualKeyCode;

	/**
	* The actual key-code generated by the platform. The DOM specification primarily uses
	* Windows-equivalent codes (hence virtualKeyCode above) but it helps to additionally
	* specify the platform-specific key-code as well.
	*/
	int nativeKeyCode;
	
	/**
	* This is a string identifying the key that was pressed. This can be generated from the
	* virtualKeyCode via the getKeyIdentifierFromVirtualKeyCode() utility function. You can
	* find the full list of key identifiers at:  http://www.w3.org/TR/DOM-Level-3-Events/keyset.html
	*/
	char keyIdentifier[20];

	/**
	* The actual text generated by this keyboard event. This is usually only a single character
	* but we're generous and cap it at a max of 4 characters.
	*/
	WebUChar text[4];

	/**
	* The text generated by this keyboard event before all modifiers except shift are applied.
	* This is used internally for working out shortcut keys. This is usually only a single
	* character but we're generous and cap it at a max of 4 characters.
	*/
	WebUChar unmodifiedText[4];

	/**
	* Whether or not the pressed key is a "system key". This is a Windows-only concept and
	* should be "false" for all non-Windows platforms. For more information, see the following
	* link: http://msdn.microsoft.com/en-us/library/ms646286(VS.85).aspx
	*/
	bool isSystemKey;
};

/**
* Utility function for generating a key identifier string from a virtual key-code.
*
* @param	virtualKeyCode	The virtual key-code to generate the key identifier from.
*
* @param	keyIdentifierResult	The string to store the result in (must be at least 20 chars).
*/
void _OSMExport getKeyIdentifierFromVirtualKeyCode(int virtualKeyCode, char** keyIdentifierResult);

}

#endif