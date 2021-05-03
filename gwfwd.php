<?php


$json = <<<'JSON'
{
  "update_id":733656607,
  "message":{
    "message_id":12632,
    "from":{
      "id":243692601,
      "is_bot":false,
      "first_name":"Ammar",
      "last_name":"Faizi",
      "username":"ammarfaizi2",
      "language_code":"en"
    },
    "chat":{
      "id":-1001226735471,
      "title":"Private Cloud",
      "type":"supergroup"
    },
    "date":1619892985,
    "reply_to_message":{
      "message_id":12626,
      "from":{
        "id":243692601,
        "is_bot":false,
        "first_name":"Ammar",
        "last_name":"Faizi",
        "username":"ammarfaizi2",
        "language_code":"en"
      },
      "chat":{
        "id":-1001226735471,
        "title":"Private Cloud",
        "type":"supergroup"
      },
      "date":1619892900,
      "text":"besok hari apa?"
    },
    "text":"/ban x",
    "entities":[
      {
        "offset":0,
        "length":6,
        "type":"bot_command"
      }
    ]
  }
}
JSON; /* end JSON */


function main(): int
{
	global $json;

	$ret  = 0;
	$sock = socket_create(AF_INET, SOCK_STREAM, 0);
	$conn = socket_connect($sock, "127.0.0.1", 55555);

	if (!$conn)
		return 1;

	$payload     = pack("n", strlen($json)).$json;
	$write_len   = socket_write($sock, $payload);
	$correct_len = strlen($payload);

	if ($write_len != $correct_len) {
		printf("socket_write failed: write_len = %d; correct_len = %d\n",
		       $write_len, $correct_len);
		$ret = 1;
	}
	socket_close($sock);
	return $ret;
}

for ($i=0; $i < 1; $i++) { 
    main();
}

// exit(main());
