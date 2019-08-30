#ifndef STYLE_H
#define STYLE_H

#define OKBTN_STYLE     "QToolButton{font: 12px \"微软雅黑\";background-color: rgb(90,115,227);border:none;border-radius:4px;color: white;}"  \
                        "QToolButton:pressed{background-color:rgb(90,115,227);}"\
                        "QToolButton:disabled{background-color:rgb(198,214,240);}"

#define CANCELBTN_STYLE "QToolButton{font: 12px \"微软雅黑\";background:rgb(104,143,205);color: rgb(255,255,255);border:none;border-radius:4px;}"  \
                        "QToolButton:pressed{background-color:rgb(104,143,205);color: white}"

#define CLOSEBTN_STYLE  "QToolButton{background-image:url(:/ui/wallet_ui/close.png);background-repeat: no-repeat;background-position: center;background-color:transparent;border:none;}"   \
                        "QToolButton:hover{background-color:rgb(208,228,255);"

#define BACKGROUNDWIDGET_STYLE  "#widget {background-color:rgba(12,18,35,219);}"

#define CONTAINERWIDGET_STYLE   "#containerwidget{background-color:rgb(255,255,255);border-radius:5px;}"  \
                                "QLabel{color:rgb(51,51,51);}"

#define TOOLBUTTON_STYLE_1      "QToolButton{font: 12px \"微软雅黑\";background-color:white; border:none;border-radius:12px;color: rgb(83,107,215);}"
                               // "QToolButton:pressed{background-color:rgb(90,115,227);color: white;}"

#define PUSHBUTTON_CHECK_STYLE  "QPushButton{font:12px \"微软雅黑\";background:transparent;border:none;color: rgb(57,68,107);}" \
                                "QPushButton::checked{color:rgb(83,107,215);border-bottom:2px solid rgb(83,107,215);}"

#define TABLEWIDGET_STYLE_1     "QTableView{background-color:rgb(255,255,255);border:none;border-radius:0px;font: 12px \"Microsoft YaHei\";color:rgb(80,80,80);}" \
                                "QHeaderView{border:none;border-bottom:1px solid #e9eef4;background-color:rgb(255,255,255);color:rgb(57,68,107,204);font:bold 12px \"Microsoft YaHei\";}" \
                                "QHeaderView:section{height:42px;border:none;background-color:rgb(255,255,255);}" \
                                "QHeaderView:section:first{border-top-left-radius:0px;}" \
                                "QHeaderView:section:last{border-top-right-radius:0px;}"


#define FUNCTIONBAR_PUSHBUTTON_STYLE    "QPushButton{color:rgb(192,199,238);text-align:center;background:transparent;border:none;font-size:14px;font-family:\"Microsoft YaHei UI Light\";}" \
                                        "QPushButton:checked{color:white;background-color:rgb(76,98,205);}"

#define BIG_BUTTON      "QToolButton{font: 11px \"Microsoft YaHei UI Light\";background-color:rgb(84,116,235); border:none;border-radius:15px;color: rgb(255, 255, 255);}" \
                                "QToolButton:pressed{background-color:rgb(70,95,191);}"

#define OKBTN_STYLE_HOVER     "QToolButton{font: 14px \"Microsoft YaHei UI Light\";background-color: rgb(84,116,235);border:0px solid white;border-radius:15px;color: white;}"  \
                        "QToolButton:hover{background-color:rgb(0,210,255);}"   \
                        "QToolButton:disabled{background-color:rgb(235,235,235);}"

#define CANCELBTN_STYLE_HOVER "QToolButton{background:rgb(229,229,229);color: white;border:0px solid white;border-radius:15px;}"  \
                        "QToolButton:hover{background-color:rgb(0,210,255);}"


#define MENU_STYLE      "QMenu {border-radius:10px;background-color:rgba(238,241,253,235);border: 0px solid red;}"\
                        "QMenu::item {border-radius:10px;border: 0px solid green;background-color:transparent;padding:5px 10px;}"\
                        "QMenu::item:selected {background-color:rgb(130,157,255);}"\
                        "QMenu::separator {height: 2px;background-color: #FCFCFC;}"\
                        "QMenu::right-arrow {padding:0px 10px;image:url(:/wallet_ui/right.png);}"

#define TEXTBROWSER_READONLY    "QTextBrowser{color:rgb(52,37,90);border:none;border-radius:10px;outline:1px solid rgb(196,191,214);outline-radius:8px;}"

#define NOACCOUNT_TIP_LABEL     "QLabel{color: rgb(83,107,215);font: 11px \"Microsoft YaHei UI Light\";}"

#define COMBOBOX_BORDER_STYLE   "QComboBox{min-height:18px;max-height:24px;background-color:rgb(255,255,255);border:1px solid #e1e4ee;color: #39446B;padding-left: 5px;font: 12px \"Microsoft YaHei UI Light\";border-top-left-radius:4px;border-bottom-left-radius:4px;border-top-right-radius:4px;border-bottom-right-radius:4px;}" \
                                "QComboBox::drop-down {width:21px;background:rgb(148,186,249);border:none;border-top-right-radius:4px;border-bottom-right-radius:4px;}" \
                                "QComboBox::down-arrow {image: url(:/ui/wallet_ui/downArrow.png);}" \
                                "QComboBox QAbstractItemView {outline: 0px;}" \
                                "QComboBox QAbstractItemView{ min-width:80px;min-height: 18px; color: #39446B;selection-color: white;background-color: rgb(255,255,255);selection-background-color: rgb(148,186,249);}"

#endif // STYLE_H
