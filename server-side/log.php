<?php
// RoomClimateLogger,  log the climate of all your rooms in a central database.
// Copyright (C) 2020  Ralf Stemmer <ralf.stemmer@gmx.net>
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

$DATE   = getdate();
$DBPATH = "/data/climatelog/climate.db";
main($DBPATH, $DATE);



function main($dbpath, $date)
{
    $data = GetInput();
    if($data != NULL)
        StoreData($dbpath, $data, $date);
    else
        echo "Fail";
}



function GetInput()
{
    $room   = "";
    $temp   = "";
    $hum    = "";

    if($_SERVER["REQUEST_METHOD"] == "GET") // FOR DEBUG - usually POST
    {
        $room = trim($_GET["r"]);
        $temp = filter_input(INPUT_GET, "t", FILTER_SANITIZE_NUMBER_FLOAT, FILTER_FLAG_ALLOW_FRACTION);
        $hum  = filter_input(INPUT_GET, "h", FILTER_SANITIZE_NUMBER_FLOAT, FILTER_FLAG_ALLOW_FRACTION);
    }
    if($_SERVER["REQUEST_METHOD"] == "POST")
    {
        $room = trim($_POST["r"]);
        $temp = filter_input(INPUT_POST, "t", FILTER_SANITIZE_NUMBER_FLOAT, FILTER_FLAG_ALLOW_FRACTION);
        $hum  = filter_input(INPUT_POST, "h", FILTER_SANITIZE_NUMBER_FLOAT, FILTER_FLAG_ALLOW_FRACTION);
    }

    if(!isset($room))
        return NULL;
    if(!isset($temp))
        return NULL;
    if(!isset($hum))
        return NULL;

    $retval = array(
            "r" => $room,
            "t" => $temp,
            "h" => $hum
        );

    return $retval;
}



// expects an array with the following keys: "r", "t", "h" for room, temperature and humidity
function StoreData($dbpath, $data, $date)
{
    $data["r"] = SQLite3::escapeString($data["r"]); // Make sure the room-entry is valid for the database

    $db  = new SQLite3($dbpath);

    $stm = $db->prepare("INSERT INTO climate (temperature, humidity, room, day, month, year, hour, minute) VALUES(:t, :h , :r, :day, :month, :year , :hour, :minute);");

    $stm->bindValue(":t",      $data["t"],       SQLITE3_FLOAT);
    $stm->bindValue(":h",      $data["h"],       SQLITE3_FLOAT);
    $stm->bindValue(":r",      $data["r"],       SQLITE3_TEXT);
    $stm->bindValue(":day",    $date["mday"],    SQLITE3_INTEGER);
    $stm->bindValue(":month",  $date["mon"],     SQLITE3_INTEGER);
    $stm->bindValue(":year",   $date["year"],    SQLITE3_INTEGER);
    $stm->bindValue(":hour",   $date["hours"],   SQLITE3_INTEGER);
    $stm->bindValue(":minute", $date["minutes"], SQLITE3_INTEGER);
    
    $stm->execute()->finalize();
    $db->close();
}


?>

