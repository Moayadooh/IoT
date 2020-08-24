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
    if(isset($_POST['insert']))
    {
        date_default_timezone_set("Asia/Muscat");
        
        $temperature = $_POST['temp_level'];
        $soil_moisture = $_POST['soil_mois_level'];
        $water_amount = $_POST['water_amount'];
        $date_time = date("Y-m-d H:i:s");
        
		$query = "INSERT INTO records (temp_level,soil_mois_level,water_amount,date_time) VALUES ('$temperature','$soil_moisture','$water_amount','$date_time');";
		
		$result = mysqli_query($link, $query);
        if(!$result)
            die("Unable to insert".mysqli_error($link));
        else
            echo "Record Inserted";
    }
	elseif(isset($_POST['retrieve']))
    {
		$query = "SELECT temp_level, soil_mois_level, water_amount FROM records;";
        $result = mysqli_query($link, $query);
        if(!$result)
            die("Unable to retrieve".mysqli_error($link));
        else
        {
			$N = 0;
			
			$x1 = 0;
			$x2 = 0;
			$y = 0;
			
			$x1Total = 0;
			$x2Total = 0;
			$yTotal = 0;
			
			$x1SquareTotal = 0;
			$x2SquareTotal = 0;
			
			$x1y = 0;
			$x2y = 0;
			$x1x2 = 0;
            while ($row = mysqli_fetch_assoc($result))
            {
				$N++;
				$x1 = $row['temp_level'];
				$x2 = $row['soil_mois_level'];
				$y = $row['water_amount'];
				
				$x1Total += $x1;
				$x1SquareTotal += $x1 * $x1;
				$x1y += $x1 * $y;
				
				$x2Total += $x2;
				$x2SquareTotal += $x2 * $x2;
				$x2y += $x2 * $y;
				
				$yTotal += $y;
				$x1x2 += $x1 * $x2;
            }
			if($N==0)
				echo "No Records";
			else
			{
				$x1Mean = $x1Total / $N;
				$x2Mean = $x2Total / $N;
				$yMean = $yTotal / $N;
				
				$x1SquareTotal = $x1SquareTotal - $x1Total * $x1Total / $N;
				$x2SquareTotal = $x2SquareTotal - $x2Total * $x2Total / $N;
				
				$x1y = $x1y - $x1Total * $yTotal / $N;
				$x2y = $x2y - $x2Total * $yTotal / $N;
				$x1x2 = $x1x2 - $x1Total * $x2Total / $N;
				
				$b1 = ($x2SquareTotal * $x1y - $x1x2 * $x2y) / ($x1SquareTotal * $x2SquareTotal - $x1x2 * $x1x2);
				$b2 = ($x1SquareTotal * $x2y - $x1x2 * $x1y) / ($x1SquareTotal * $x2SquareTotal - $x1x2 * $x1x2);
				$b0 = $yMean - $b1 * $x1Mean - $b2 * $x2Mean;
				
				$query = "SELECT avg(temp_level) AS average FROM records ORDER BY trend_id DESC LIMIT 5;";
				$result = mysqli_query($link, $query);
				if(!$result)
					die("Unable to retrieve".mysqli_error($link));
				$row = mysqli_fetch_assoc($result);
				$x1 = $row['average'];
				
				$query = "SELECT avg(soil_mois_level) AS average FROM records ORDER BY trend_id DESC LIMIT 5;";
				$result = mysqli_query($link, $query);
				if(!$result)
					die("Unable to retrieve".mysqli_error($link));
				$row = mysqli_fetch_assoc($result);
				$x2 = $row['average'];
				
				$y = $b0 + $b1 * $x1 + $b2 * $x2;
				echo json_encode(array("water_amount"=>$y));
			}
        }
    }
}
mysqli_close($link);
?>		