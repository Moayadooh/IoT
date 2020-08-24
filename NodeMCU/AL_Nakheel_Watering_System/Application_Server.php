<?php
$hostname = "127.0.0.1";
$username = "root";
$password = "root";
$dbname = "iot";

$link = mysqli_connect($hostname, $username, $password, $dbname);
if(!$link)
	die("Unable to connect".mysqli_error($link));
else
{
	if($_SERVER['REQUEST_METHOD']=='POST')
	{
		$response = array();
		$query = "SELECT * FROM records ORDER BY trend_id DESC LIMIT 20;";
		$result = mysqli_query($link, $query);
		if(!$result)
			die("Unable to retrieve".mysqli_error($link));
		else
		{
			while ($row = mysqli_fetch_assoc($result))
			{
				$response[] = $row;
			}
			echo json_encode($response);
		}
	}
}
mysqli_close($link);
?>