<?php


$json = <<<'JSON'
{
    "update_id": 346134787,
    "message": {
        "message_id": 4007,
        "from": {
            "id": 243692601,
            "is_bot": false,
            "first_name": "Ammar",
            "last_name": "Faizi",
            "username": "ammarfaizi2",
            "language_code": "en"
        },
        "chat": {
            "id": -1001347566306,
            "title": "GNU/Weeb Test Driven Development",
            "username": "GNUWeebTDD",
            "type": "supergroup"
        },
        "date": 1621503523,
        "reply_to_message": {
            "message_id": 4006,
            "from": {
                "id": 243692601,
                "is_bot": false,
                "first_name": "Ammar",
                "last_name": "Faizi",
                "username": "ammarfaizi2",
                "language_code": "en"
            },
            "chat": {
                "id": -1001347566306,
                "title": "GNU/Weeb Test Driven Development",
                "username": "GNUWeebTDD",
                "type": "supergroup"
            },
            "date": 1621503517,
            "text": "test"
        },
        "text": ".delete"
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
