const char CSS_MAIN[] PROGMEM = R"=====(
body {
	display: flex;
	flex-direction: column;
	background-color: gainsboro;
	margin: 0;
}

body * {
	background: linear-gradient(180deg, #a5d6a770, #00000000);
	box-shadow: 5px 5px 20px #292323;
	border-radius: 10px;
	display: flex;
	flex: auto;
	flex-direction: column;
	font-family: Verdana, Geneva, sans-serif;
	/*font: menu;*/
	line-height: 1.5em;
	font-size: calc((1vmin + 1vmax) * 1.25);
	font-weight: 600;
}

/* Style the top navigation bar */
.ui_tab_panel {
	border: solid;
	border-width: 0 0 10;
	border-bottom-color: #608b61;
	overflow: hidden;
	background-color: #333;
}

/* Style the topnav links */
.ui_tab_panel button {
	flex: none;
	font-size: 150%;
	background-color: #666;
	color: #f2f2f2;
	padding: 14px 16px;
	border: none;
	margin: 1px;
}

/* Change color on hover */
.ui_tab_panel button:hover {
	cursor: pointer;
	background-color: #ddd;
	color: black;
}

.ui_tab_panel button:focus {
	background-color: #0A0;
	outline: none;
}

.ui_tab_panel:empty { display: none }

/* Footer */
.ui_footer {
	padding: 20px;
	text-align: center;
	background: #ddd;
	margin-top: 20px;
}

.ui_title {
	/*min-width: max-content;*/
	font-size: 150%;
	/*width: fit-content;*/
	overflow: hidden;
	background: linear-gradient(180deg, #96ca93, transparent);
	text-align: center;
	color: #000;
	padding: 1%;
	border: outset;
	border-width: 5px;
	/*margin-right: auto;*/
}

.ui_box {
	/*min-width: max-content;*/
	overflow: hidden;
	/*width: min-content;*/
	background-color: #3333;
	padding: 1px;
}

.ui_text {
	white-space: nowrap;
	background-color: #444;
	color: #fff;
	padding: 10px;
	margin: 2px;
	border: none;
}

.ui_button {
	place-items: center;
	background-color: #666;
	color: #f2f2f2;
	padding: 10px;
	margin: 2px;
	border: none;
}

/* Change color on hover */
.ui_button:hover {
	cursor: pointer;
	box-shadow: 0px 0px 400px 200px rgba(155,255,155,0.5) inset;
	color: black;
}

.ui_button:focus {
	outline: none;
}

.ui_button:active{
	box-shadow: 0px 0px 400px 200px rgba(255,255,255,0.5) inset;
	color: black;
}

.ui_button:disabled{
	box-shadow: 0px 0px 400px 200px rgba(0,0,0,0.5) inset;
	color: black;
}

.ui_button:empty::after {
    content: ".";
    visibility: hidden;
}

.ui_checkbox {
	width: 2em;
	height: 2em;
	margin: auto;
	flex-grow: 1;
}

.ui_field {
	flex-direction: row;
	background-color: #fff;
	width: 100%;
	color: #000;
	padding: 10px;
	margin: 2px;
	border: none;
}

.ui_field:disabled {
	box-shadow: 0px 0px 400px 200px rgba(0,0,0,0.5) inset;
	color: black;
}

.ui_range {
	text-align: center;
	background-color: #333;
	color: #fff;
	width: 100%;
}

.ui_range_output {
	white-space: nowrap;
	background-color: #444;
	color: #fff;
	/*padding: 2px;*/
	margin: 2px;
	border: none;
}

.ui_dialog_background {
	position: fixed;
	width: 100%;
	height: 100%;
	backdrop-filter: blur(10px);
	animation: ui_dialog_background 0.5s;
	/*background: radial-gradient(#9dff00, transparent);*/
}

@keyframes ui_dialog_background {
	from {opacity:0;}
	to {opacity:1;}
}

.ui_dialog {
	background: linear-gradient(180deg, #0089ff, #00ff9585);
	position: absolute;
	width: 90%;
	left: 5%;
	/*top: 25%;*/
	/*min-height: 15em;*/
}

.ui_dialog::before {
	position: initial;
}

.connection_widget {
	position: fixed;
	bottom: 0px;
	right: 0px;
	color: #fff;
	width: 100%;
	flex-direction: row;
	background: #111;
	opacity: 0.75;
	font-size: 50%;
}

.ui_loading {
	flex: none;
	border-radius: 50%;
	margin: 8px;
	border: 2.5em double #fff;
	border-color: #ffeb3b77 #79554877 #ffeb3b77 #79554877;
	animation: ui_loading 1s infinite;
	background: none;
	box-shadow: none;
}

@keyframes ui_loading {
	from {transform: rotate(0);}
	to {transform: rotate(360deg);}
}

@media screen and (orientation: portrait) {
	.ui_tab_panel {
		flex-flow: wrap;
	}
	
	.ui_tab_panel button {
		flex: auto !important;
	}
	
	body * {
		/*min-width: 0px !important;*/
		/*flex: auto !important;*/
	}
}
)=====";
