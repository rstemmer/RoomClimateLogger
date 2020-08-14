//function reqListener()
//{
//    window.console && console.log(this.responseText);
//}

window.onload = function()
{
    window.console && console.log("request…");
    var request = new XMLHttpRequest()
    request.onload = function()
    {
        window.console && console.log(this.responseText);
    };
    request.open("get", "get.php", true); // true -> non-blocking
    request.send();
}
