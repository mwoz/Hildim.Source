// SciTE - Scintilla based Text Editor
/** @file SingleThreadExtension.cxx
 ** Extension that wraps another extension so that OnExecute calls
 ** are always funneled to the initial thread.
 **/
// Copyright 1998-2004 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.


// For the single case of LuaExtension where this is needed at present,
// it might have been simpler (well, a little less tedious and very
// slightly more efficient) to make this inherit from LuaExtension, and
// only override Initialise, Finalise, and OnExecute.  Or to do it with
// an #ifdef in LuaExtension.  But I did it this way so that other
// extensions can leverage it without needing to do anything special.

#include "Platform.h" //!-add-[no_wornings]
#include <string>

#include "Scintilla.h"
#include "GUI.h"
#include "SingleThreadExtension.h"

// Is it true that only OnExecute needs to be protected / serialized?
// Lua does not support macros, but does OnMacro also need it?  Others?

// For OnExecute and any other messages that needed to be dispatched,
//   WPARAM will point to the wrapped extension
//   LPARAM will hold the argument(s)
//   Return value is 0 for false (continue), 1 for true (handled / stop)

enum {
	STE_WM_ONEXECUTE = 2001
};

LRESULT PASCAL SingleThreadExtension::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg == STE_WM_ONEXECUTE) {
		Extension *extension = reinterpret_cast<Extension *>(wParam);
		return extension->OnExecute(reinterpret_cast<char *>(lParam));
	}

	WNDPROC lpPrevWndProc = reinterpret_cast<WNDPROC>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	if (lpPrevWndProc)
		return ::CallWindowProc(lpPrevWndProc, hwnd, uMsg, wParam, lParam);

	return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
}

bool SingleThreadExtension::Initialise(ExtensionAPI *host_) {
	hwndDispatcher = CreateWindow(
		TEXT("STATIC"), TEXT("SciTE_SingleThreadExtension_Dispatcher"),
		0, 0, 0, 0, 0, 0, 0, GetModuleHandle(NULL), 0
	);

	LONG_PTR subclassedProc = SetWindowLongPtr(hwndDispatcher, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc));
	SetWindowLongPtr(hwndDispatcher, GWLP_USERDATA, subclassedProc);

	return ext->Initialise(host_);
}
void SingleThreadExtension::PostInit(void* h){
	ext->PostInit(h);
}
void* SingleThreadExtension::GetLuaState(){
	return ext->GetLuaState();
}

bool SingleThreadExtension::Finalise() {
	ext->Finalise();

	if (hwndDispatcher) {
		DestroyWindow(hwndDispatcher);
		hwndDispatcher = NULL;
	}

	return false;
}

bool SingleThreadExtension::Clear() {
	return ext->Clear();
}

bool SingleThreadExtension::Load(const char *filename) {
	return ext->Load(filename);
}

bool SingleThreadExtension::InitBuffer(int index) {
	return ext->InitBuffer(index);
}

bool SingleThreadExtension::ActivateBuffer(int index) {
	return ext->ActivateBuffer(index);
}

bool SingleThreadExtension::RemoveBuffer(int index) {
	return ext->RemoveBuffer(index);
}

bool SingleThreadExtension::OnOpen(const char *filename) {
	return ext->OnOpen(filename);
}

bool SingleThreadExtension::OnSwitchFile(const char *filename) {
	return ext->OnSwitchFile(filename);
}

bool SingleThreadExtension::OnBeforeSave(const char *filename) {
	return ext->OnBeforeSave(filename);
}

bool SingleThreadExtension::OnSave(const char *filename) {
	return ext->OnSave(filename);
}

bool SingleThreadExtension::OnChar(char c) {
	return ext->OnChar(c);
}

bool SingleThreadExtension::OnExecute(const char *cmd) {
	return (SendMessage(hwndDispatcher, STE_WM_ONEXECUTE, reinterpret_cast<WPARAM>(ext), reinterpret_cast<LPARAM>(cmd)) != 0);
}

bool SingleThreadExtension::OnSavePointReached() {
	return ext->OnSavePointReached();
}

bool SingleThreadExtension::OnSavePointLeft() {
	return ext->OnSavePointLeft();
}

bool SingleThreadExtension::OnStyle(unsigned int p, int q, int r, StyleWriter *s) {
	return ext->OnStyle(p,q,r,s);
}

/*!
bool SingleThreadExtension::OnDoubleClick() {
	return ext->OnDoubleClick();
}
*/
//!-start-[OnDoubleClick]
bool SingleThreadExtension::OnDoubleClick(int modifiers){
	return ext->OnDoubleClick(modifiers);
}
//!-end-[OnDoubleClick]

//!-start-[OnClick]
bool SingleThreadExtension::OnClick(int modifiers){
	return ext->OnClick(modifiers);
}
//!-end-[OnClick]

//!-start-[OnHotSpotReleaseClick]
bool SingleThreadExtension::OnHotSpotReleaseClick(int modifiers){
	return ext->OnHotSpotReleaseClick(modifiers);
}
//!-end-[OnHotSpotReleaseClick]

//!-start-[OnMouseButtonUp]
bool SingleThreadExtension::OnMouseButtonUp(int modifiers){
	return ext->OnMouseButtonUp(modifiers);
}
//!-end-[OnMouseButtonUp]

bool SingleThreadExtension::OnUpdateUI(bool bModified, bool bSelChange, int flag) {
	return ext->OnUpdateUI(bModified, bSelChange, flag);
}

bool SingleThreadExtension::OnMarginClick(unsigned int margin, unsigned int modif, long line) {
	return ext->OnMarginClick(margin, modif, line);
}

bool SingleThreadExtension::OnMacro(const char *p, const char *q) {
	return ext->OnMacro(p,q);
}

/*!
bool SingleThreadExtension::OnUserListSelection(int listType, const char *selection) {
	return ext->OnUserListSelection(listType, selection);
}
*/
//!-start-[UserListItemID]
bool SingleThreadExtension::OnUserListSelection(int listType, const char *selection, int id) {
	return ext->OnUserListSelection(listType, selection, id);
}
bool SingleThreadExtension::OnNavigation(const char *item){
	return ext->OnNavigation(item);
}
//!-end-[UserListItemID]

//!-start-[OnMenuCommand]
bool SingleThreadExtension::OnMenuCommand(int cmd, int source) {
	return ext->OnMenuCommand(cmd, source);						 
}
//!-end-[OnMenuCommand]

//!-start-[OnSendEditor]
const char *SingleThreadExtension::OnSendEditor(unsigned int msg, unsigned int wp, const char *lp) {
	return ext->OnSendEditor(msg, wp, lp);
}

const char *SingleThreadExtension::OnSendEditor(unsigned int msg, unsigned int wp, long lp) {
	return ext->OnSendEditor(msg, wp, lp);
}
//!-end-[OnSendEditor]

bool SingleThreadExtension::SendProperty(const char *prop) {
	return ext->SendProperty(prop);
}

//!-start-[OnKey]
#if !defined(GTK)
bool SingleThreadExtension::OnKey(int keyval, int modifiers, char ch) {
	return ext->OnKey(keyval, modifiers, ch);
}
#else
//!-end-[OnKey]
bool SingleThreadExtension::OnKey(int keyval, int modifiers) {
	return ext->OnKey(keyval, modifiers);
}
#endif //!-add-[OnKey]
bool SingleThreadExtension::OnDwellStart(int pos, const char *word) {
	return ext->OnDwellStart(pos, word);
}

bool SingleThreadExtension::OnClose(const char *filename) {
	return ext->OnClose(filename);
}

bool SingleThreadExtension::OnColorized(unsigned int wp, unsigned int lp){
	return ext->OnColorized(wp,lp);
}

const char *SingleThreadExtension::OnContextMenu(unsigned int msg, unsigned int wp, const char *lp) {
	return ext->OnContextMenu(msg, wp, lp);
}

bool SingleThreadExtension::OnFindCompleted(){
	return ext->OnFindCompleted();
}

bool SingleThreadExtension::OnIdle(){
	return ext->OnIdle();
}

bool SingleThreadExtension::OnLayOutNotify(const char *command) {
	return ext->OnLayOutNotify(command);
}

bool SingleThreadExtension::OnGeneratedHotKey(long hotkey) {
	return ext->OnGeneratedHotKey(hotkey);
}

void SingleThreadExtension::DoReboot(){
	ext->DoReboot();
}

void SingleThreadExtension::DoLua(const char * c){
	ext->DoLua(c);
}
void SingleThreadExtension::OnMouseHook(int x, int y){
	ext->OnMouseHook(x, y);
}
bool SingleThreadExtension::OnDrawClipboard(int flag){
	return ext->OnDrawClipboard(flag);
}
