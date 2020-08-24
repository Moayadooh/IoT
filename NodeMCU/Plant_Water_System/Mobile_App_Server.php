<?php
$hostname = $_POST['hostname'];
$username = $_POST['username'];
$password = $_POST['password'];
$dbname = $_POST['dbname'];

$link = mysqli_connect($hostname, $username, $password, $dbname);
if(!$link)
	die("Unable to connect".mysqli_error($link));
else
{
	if(isset($_POST['RETRIEVE_RECORDS']))
	{
		$response = array();
		$query = "SELECT * FROM trends ORDER BY trend_id DESC LIMIT 20;";
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
	elseif(isset($_POST['STORE_PLANT_AGE']))
	{
		$plantID = $_POST['plantID'];
		$plantAge = $_POST['plantAge'];
		$nextYearDate = $_POST['nextYearDate'];
		
		$sql = "INSERT INTO plants (plant_id,plant_age) VALUES ('$plantID','$plantAge') ON DUPLICATE KEY UPDATE plant_age='$plantAge', next_update_date='$nextYearDate';";
		$result = mysqli_query($link, $sql);
		
		if(!$result)
			die("Unable to store the plant age!".mysqli_error($link));
		else
			echo "Plant Age Updated Successfully";
	}
	elseif(isset($_POST['RETRIEVE_PLANTS_NAMES']))
	{
		$response = array();
		$query = "SELECT * FROM plants;";
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
	elseif(isset($_POST['RETRIEVE_PLANT_AGE']))
	{
		$plantID = $_POST['plantID'];
		
		$sql = "SELECT * FROM plants where plant_id = '$plantID';";
		$result = mysqli_query($link, $sql);
		
		if(!$result)
			die("Unable to retrieve the plant age!".mysqli_error($link));
		$row = mysqli_fetch_assoc($result);
		$response[] = $row;
		echo json_encode($response);
	}
}
mysqli_close($link);
?>