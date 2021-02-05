const char HTML_INDEX[] PROGMEM = R"=====(
<head>
	<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
	<title>ElektroBOX UI:</title>
	<meta name="viewport" content="width=device-width, initial-scale=1" />
	<link rel="shortcut icon" href="data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAMAAAAoLQ9TAAAABGdBTUEAALGPC/xhBQAAACBjSFJNAAB6JgAAgIQAAPoAAACA6AAAdTAAAOpgAAA6mAAAF3CculE8AAAAPFBMVEUAAACA1VWR21qQ2liR3FqR3FqS3VuR3VqR3VuR3VqO21mS21uS3FqS3FqS21uJ2GKQ21qR3FuR3FoAAAB/3Gu7AAAAEnRSTlMABoA3kPBwz8i5Kzioxg4NVcU3uEJHAAAAAWJLR0QAiAUdSAAAAAlwSFlzAAAN1wAADdcBQiibeAAAAAd0SU1FB+EFEhcEM+HpYwQAAABYSURBVBjThY/JDsAgCESt4lpX/v9jLQZJ6qF9t3khAyj1xXUKbQ4BVowDwqOYgExkkW4iY6lPaF06RqM8YItOuRbMaz6xjbsusDAW/drplBg47jP696cXE8bPA1eUDeK2AAAAJXRFWHRkYXRlOmNyZWF0ZQAyMDE3LTA1LTE4VDIzOjA0OjUxKzAyOjAwxE59ewAAACV0RVh0ZGF0ZTptb2RpZnkAMjAxNy0wNS0xOFQyMzowNDo1MSswMjowMLUTxccAAAAZdEVYdFNvZnR3YXJlAHd3dy5pbmtzY2FwZS5vcmeb7jwaAAAAAElFTkSuQmCC"/>
	<link rel="stylesheet" href="/css/style.css" />
	<script src="/js/main.js"></script>
</head>

<body id = "body" onload="main();">
	<div id = "main"></div>
	<div style = "visibility: hidden;">inv</div>
	<div class = "connection_widget">
		<div id = "connection_title" style = "background: none">connection:</div>
		<div id = "connection_value" style = "color: orange;">not established</div></div>
	</div>
</body>
)=====";
