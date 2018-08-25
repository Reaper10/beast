'use strict';

// == Application Menu ==
const file_menu = [
  { label: _('&Open...'),		role: 'open-file',		accelerator: 'Ctrl+O', },
  { label: _('&Quit'),			role: 'quit-app',		accelerator: 'Shift+Ctrl+Q', },
];
const view_menu = [
  { label: _('Toggle &Fullscreen'), 	 role: 'toggle-fulscreen', 	accelerator: 'F11', },
];
const help_menu = [
  { label: _('&About...'),		role: 'about-dialog',		},
];
const menubar_menus = [
  { label: _('&File'), 			submenu: file_menu,		accelerator: 'Alt+F' },
  { label: _('&View'), 			submenu: view_menu,		accelerator: 'Alt+W' },
  { label: _('&Help'), 			submenu: help_menu,		accelerator: 'Alt+H' },
];

// add 'click' handlers to menu templates
function assign_click (item, func) {
  if (Array.isArray (item)) {
    for (let i = 0; i < item.length; i++)
      assign_click (item[i], func);
  } else if (item.submenu !== undefined) {
    assign_click (item.submenu, func);
  } else if (item.click === undefined)
    item.click = func;
}

// assign global app menu
assign_click (menubar_menus, (menuitem, _focusedBrowserWindow, _event) => {
  menu_command (menuitem.role, menuitem.data);
});
module.exports.build_menubar = function () {
  return Electron.Menu.buildFromTemplate (menubar_menus);
};

// handle menu activations
function menu_command (role, _data) {
  const BrowserWindow = Electron.getCurrentWindow(); // http://electron.atom.io/docs/api/browser-window/
  switch (role) {
  case 'about-dialog':
    Shell.show_about_dialog = !Shell.show_about_dialog;
    break;
  case 'toggle-fulscreen':
    BrowserWindow.setFullScreen (!BrowserWindow.isFullScreen());
    break;
  case 'quit-app':
    Electron.app.quit();
    return false;
  case 'open-file':
    Electron.dialog.showOpenDialog ({
      properties: ['openFile', 'openDirectory', 'multiSelections'],
    }, (result) => {
      console.log ('open-file: ' + result);
    });
    break;
  default:
    console.log ('unhandled menu command: ' + role);
    break;
  }
}
