const char JS_MAIN[] PROGMEM = R"=====(
const is_touch = ("ontouchstart" in document.documentElement) ? true : false;
		
var websock;

const E_TAB = 0;
const E_BOX = 1;
const E_TEXT = 2;
const E_RADIO = 3;
const E_BUTTON = 4;
const E_SWITCHER = 5;
const E_APPLIER = 6;
const E_FIELD = 7;
const E_DATE_FIELD = 8;
const E_TIME_FIELD = 9;
const E_RANGE = 10;
const E_DIALOG = 11;
const GET_TIME = 12;
const FRAME = 13;
const E_LOADING = 14;

//Парсер вхідних повідомлень
//Повертає ui елемент формату:
//ui_element = {type:integer, id:string, parent:string, attributes:{attrib0:string, ..., attribN:string}}
function parse_message(str, end)
{
	while (true) {
		if (end.val >= str.length) {
			return "";
		} else if (str[end.val] == ':') {
			end.val++;
			break;
		}
		break;
	}

	var begin = end.val;
		
	for (; end.val < str.length; end.val++)
		if (str[end.val] == ':' && str[end.val-1] != '\\') break;
	
	return str.substring(begin, end.val);
}

function parse_ui_messages(str)
{
	begin = {val : 0};
	
	var messages = [];
	var message;
	
	while (true) {
		var key = parse_message(str, begin), val = parse_message(str, begin);
		switch (key) {
		case "type":
			if (message)
				messages.push(message);
			message = {attributes : {}};
			message.type = parseInt(val);
			
			break;
		
		case "id":
			if (message) {
				if (message.id) {
					messages.push(message);
					message = {attributes : {}};
				}
				message.id = val;
			} else {
				message = {attributes : {}};
				message.id = val;
			}
			break;
										
		case "parent":
			message.parent = val;
			break;
			
		case "log":
			console.log(val.replace(/\\:/g, ":"));
			break;
		
		case "":
			if (message)
				messages.push(message);
			return messages;

		default: 
			message.attributes[key] = unescape(val);
			break;
		}
	}
}

//Встановити публічні атрібути які справедливі для всіх ui елементів		
function set_generic_web_attributes(web_element, attributes)
{
	if (!attributes) return;
	for (const [key, value] of Object.entries(attributes)) {
		
		switch(key) {
		case "dir": //Напрямок піделементів (горизонтальний(по дефолту)/вертикальний)
			if (value == "v") web_element.style.flexDirection = "column";
			else web_element.style.flexDirection = "row";
			break;
		
		case "wrap": //Автоперенесення піделементів
			if (value == "true") {
				if (web_element.className == "ui_text") { //Костиль
					web_element.style.whiteSpace = "normal";
					web_element.style.textAlign = "left";
				} else {
					web_element.style.flexWrap = "wrap";
				}
			}
			else web_element.style.flexWrap = "nowrap";
			break;
		
		case "align": //Вирівнювання піделементів по відношенню до напрямку (Start / center / end)
			if (value == "up" || value == "left")
				web_element.style.alignItems = "flex-start";
			else if (value == "down" || value == "right")
				web_element.style.alignItems = "flex-end";
			else if (value == "center")
				web_element.style.alignItems = "center";
			else
				web_element.style.alignItems = "unset";
			break;
		
		case "w": //Ширина елементу в одиницях em.
			if (value == "fill")
				web_element.style.width = "100%"
			else if (value == "content")
				web_element.style.flex = "none";			
			else
				//web_element.style.flex = "none";
				web_element.style.minWidth = parseFloat(value) + "em";
			break;
			
		case "h": //Висота елементу в одиницях em.
			if (value == "fill")
				web_element.style.height= "100%"
			else if (value == "content")
				web_element.style.flex = "none";			
			else
				//web_element.style.flex = "none";
				web_element.style.minHeight = parseFloat(value) + "em";
			break;
			
		case "text": //Встановити текстове значеня
			if (web_element.className == "ui_tab") //Костиль
				get_web_element(web_element.id + "_tab").innerHTML = value;
			else
				web_element.innerHTML = value;
			break;
			
		case "size": //Встановити текстове значеня
				web_element.style.fontSize = parseFloat(value) + "%";
			break;
		
		case "value":
			if (document.activeElement != web_element || web_element.force_update) {
				if (web_element.className == "ui_range_output") //Костиль
					web_element.nextElementSibling.value = value;
				web_element.value = value;
				web_element.force_update = false;
			}
			break;
		
		case "backcolor":
			web_element.style.backgroundColor = value;
			break;
		
		case "textcolor":
			web_element.style.color = value;
			break;	
			
		case "display": //Відображати, чи ні.
			if (web_element.className == "ui_dialog") //костиль
				web_element = get_web_element(web_element.id + "_background");
				
			if (value == "false") web_element.style.display = "none";
			else web_element.style.display = "flex";
			break;
			
		case "disabled":
			if (value == "true") web_element.disabled = true;
			else web_element.disabled = false;
			break;
		
		default: break;
		}
	}
}

//Отримати атрібут з його видаленням зі списку атрібутив
function extrude_ui_attribute(ui_element, attribute)
{
	var extruded_attribute = ui_element.attributes[attribute];
	delete ui_element.attributes[attribute];
	return extruded_attribute;
}

function get_web_element(id)
{
	return document.getElementById(id);
}

//Спробувати вставити новий web елемент
//Інакше - обновити існуюучий
function try_insert_web_element(parent, web_element, attributes)
{
	set_generic_web_attributes(web_element, attributes);
	var e = get_web_element(web_element.id);
	if (!e)
		get_web_element(parent).appendChild(web_element);
	else 
		e.replaceWith(web_element);
}

function open_tab(evt, tab_id)
{
	var tab = get_web_element(tab_id);
	var i, tabcontent, tablinks;
	
	tabcontent = document.getElementsByClassName(tab.className);
	for (i = 0; i < tabcontent.length; i++) {
		tabcontent[i].style.display = "none";
	}
	
	tablinks = evt.currentTarget.parentNode.children;
	for (i = 0; i < tablinks.length; i++) {
		tablinks[i].className = tablinks[i].className.replace(" active", "");
		tablinks[i].style.removeProperty("background-color");
	}
	
	tab.style.display = "flex";
	evt.currentTarget.className += " active";
	//evt.currentTarget.style.backgroundColor = getComputedStyle(evt.currentTarget).backgroundColor;
	evt.currentTarget.style.backgroundColor = "#0A0"; //hardcoded shit
}

function parse_messages(messages)
{
	parse_ui_messages(messages).forEach((ui_element) => {
		//Отримати всі аттрібути
		//message.attr = parse_ui_attributes(message.attr);
		//console.log(message.attr);
		//Якщо немає parent атрібута, то встановити body parent

		console.log(ui_element);
		switch(ui_element.type) {

		case E_TAB:
			var selected_tab = ui_element.attributes.selected;
			var panel = get_web_element(ui_element.attributes.panel);
			if (!panel) {
				panel = document.createElement('div');
				panel.id = ui_element.attributes.panel;
				panel.className = "ui_tab_panel";

				//Якщо атрибута не існує, або ж він дорівнює h, то встановити напрямок як h(горизонтальний).
				if (!ui_element.attributes.panel_align || ui_element.attributes.panel_align == "h") panel.style.flexDirection = "row";

				try_insert_web_element(ui_element.parent, panel);
			}
			
			var tab = document.createElement('button');
			tab.id = ui_element.id + "_tab";
			if (ui_element.attributes.tab_align == "right" && panel.style.flexDirection == "row") tab.style.marginLeft = "auto";
			tab.setAttribute("onclick", "open_tab(event, '" + ui_element.id + "')");			
			tab.innerHTML = extrude_ui_attribute(ui_element, "text");
			try_insert_web_element(panel.id, tab);
			
			var tab_content = document.createElement('div');
			tab_content.id = ui_element.id;
			tab_content.style.display = "none";
			tab_content.className = panel.id;
			try_insert_web_element(ui_element.parent, tab_content, ui_element.attributes);
			
			//Сфокусуватися та активувати перший таб у списку.
			if (selected_tab == "true") {
				tab.focus();
				tab.click();
			}
			
			break;
			
		case E_BOX:
			var title;
			if (ui_element.attributes.text) {
				title = document.createElement('div');
				title.id = ui_element.id + "_title";
				title.className = "ui_title";
				title.style.flexDirection = "column";
				if (ui_element.attributes.fill == "true") title.style.flex = "100%";
				title.innerHTML = extrude_ui_attribute(ui_element, "text");
				try_insert_web_element(ui_element.parent, title);
			}
		
			var box = document.createElement('div');
			box.id = ui_element.id;
			box.className = "ui_box";
			
			try_insert_web_element(title ? title.id : ui_element.parent, box, ui_element.attributes);
			
			break;
			
		case E_DIALOG:
			var background = document.createElement('div'); //blurred background for dialog
			background.id = ui_element.id + "_background";
			background.className = "ui_dialog_background";
			try_insert_web_element("main", background);
			
			var dialog = document.createElement('div');
			dialog.id = ui_element.id;
			dialog.className = "ui_dialog";
			try_insert_web_element(background.id, dialog, ui_element.attributes);
			
			break;
			
		case E_LOADING:
			var loading = document.createElement('div'); //blurred background for dialog
			loading.id = ui_element.id;
			loading.className = "ui_loading";
			try_insert_web_element(ui_element.parent, loading);
			break;
		
		case E_TEXT:
			var text = document.createElement('div');
			text.id = ui_element.id;
			text.className = "ui_text";
			try_insert_web_element(ui_element.parent, text, ui_element.attributes);
			
			break;
		
		case E_RADIO:
		case E_BUTTON:
		case E_SWITCHER:
		case E_APPLIER:
			var button = document.createElement('button');
			button.id = ui_element.id;
			button.className = "ui_button";
			button.setAttribute("onclick", "send_button(this.id)"); 
			try_insert_web_element(ui_element.parent, button, ui_element.attributes);
			
			break;

		case E_DATE_FIELD:
			var date_field = true;
		case E_TIME_FIELD:
			var time_field = true;
		case E_FIELD:
			var field = document.createElement('input');
			field.id = ui_element.id;
			field.className = "ui_field";
			field.type = date_field ? "date" : (time_field ? "time" : "field");
			field.setAttribute("onfocus", "this.oldval = this.value");
			field.setAttribute("onchange", "this.force_update = true; try {send_field(this.id, this.value)} finally {this.value = this.oldval}");
			field.setAttribute("size", "4");
			try_insert_web_element(ui_element.parent, field, ui_element.attributes);
			
			break;
			
		//redesign this horrible shit
		case E_RANGE:
			var range = document.createElement('div');
			range.id = ui_element.id + "_range";
			range.className = "ui_range";
			range.style.flexDirection = "column";
			try_insert_web_element(ui_element.parent, range, ui_element.attributes);
				
			var output = document.createElement('output')
			output.value = ui_element.attributes.value;
			output.id = ui_element.id;
			output.className = "ui_range_output";
			try_insert_web_element(range.id, output);
			
			var slider = document.createElement('input');
			slider.id = ui_element.id + "_slider";
			slider.className = "ui_range_output";
			slider.type = "range";
			slider.value = ui_element.attributes.value;
			slider.min = ui_element.attributes.min;
			slider.max = ui_element.attributes.max;
			slider.setAttribute("oninput", "this.previousElementSibling.value = this.value");
			//slider.setAttribute(is_touch ? "ontouchend" : "onmouseup", "send_range(this.id, this.value)");
			slider.setAttribute(is_touch ? "ontouchstart" : "onmousedown", "this.oldval = this.value");
			slider.setAttribute(is_touch ? "ontouchend" : "onmouseup", "try {send_range(previousElementSibling.id, this.value)} finally {this.blur(); this.previousElementSibling.value = this.value = this.oldval}");
			try_insert_web_element(range.id, slider);
			
			break;
			
		case GET_TIME:
			send_time(ui_element.id);
			break;
			
		case FRAME:
			send_frame_confirmation(ui_element.id);
			break;
			
		default:
			if (!ui_element.type)
				set_generic_web_attributes(get_web_element(ui_element.id), ui_element.attributes);
			else
				console.error("Unknown type or event", ui_element);
			break;
		}
	});
};

function escape(str)
{
	return str.replace(/:/g, "\\:"); //better browser compatiblity
}

function unescape(str)
{
	return str.replace(/\\:/g, ":").replace(/\n/g, "<br>");
}

function send_button(id) {
	websock.send(id + ":");
}

function send_field(id, value) {
	websock.send(id + ":" + escape(value) + ":");
}

function update_range(id, out)
{
	out.value = value;
}

function send_range(id, value) {
	websock.send(id + ":" + value + ":");
}

function send_time(id) {
	var d = new Date();
	var unix_utc_timestamp = Math.round((d.getTime()-(d.getTimezoneOffset()*60000)) / 1000);
	websock.send(id + ":" + unix_utc_timestamp + ":");
}

function send_frame_confirmation(id) {
	setTimeout(function() { websock.send(id + ":") }, 100);
}

//function auto_reconnect()
//{
	
//}

function no_connection()
{
	var connection = get_web_element("connection_value");
	connection.innerHTML = "disconnected";
	//setTimeout(auto_reconnect(), 2000);
	connection.style.color = "red";
}

function connected()
{
	var connection = get_web_element("connection_value");
	connection.innerHTML = "connected"
	connection.style.color = "lightgreen";
}

function main()
{
	websock = new WebSocket("ws://" + window.location.hostname + "/ws");
	
	websock.onopen = function(evt) {
		console.log("websock open");
		connected();
	};

	websock.onclose = function(evt) {
		console.log("websock close");
		no_connection();
	};

	websock.onerror = function(evt) {
		console.log(evt);
		no_connection();
	};

	websock.onmessage = function(evt) {
		parse_messages(evt.data);
	}
}
)=====";
