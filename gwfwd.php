<?php


$json = <<<'JSON'
{
  "update_id":733656170,
  "message":{
    "message_id":3366,
    "from":{
      "id":243692601,
      "is_bot":false,
      "first_name":"Ammar",
      "last_name":"F",
      "username":"ammarfaizi2",
      "language_code":"en"
    },
    "chat":{
      "id":-508064181,
      "title":"massive",
      "type":"group",
      "all_members_are_administrators":true
    },
    "date":1617809408,
    "text":"/debug",
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

for ($i=0; $i < 100; $i++) { 
    main();
}

// exit(main());
