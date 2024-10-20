<?php

/** PHPExcel to read and write excel files. **/
include $_SERVER['DOCUMENT_ROOT'] . '/php/PHPExcelLast/Classes/PHPExcel/IOFactory.php';
// Set default time zone.
date_default_timezone_set('America/Chicago');

chdir('../');

// Calculate cow value for single cow.
// Open a file and dump the user input into it in csv format to be used by ./calculate.
$filename = './input/input.csv';
$file = fopen($filename, 'w+') or die('Cannot open file ' . $filename);
fputcsv($file, $_POST, ',') or die('Could not update ' . $filename);
fclose($file);

// Call the c executable to do calculations.
$lastLine = exec('./c/calculate -n 1 -i ./input/input.csv', $output, $retval_cow);

/** Check retval to see if the execution was OK. **/
if ($retval_cow == 0) {
	/** We are good. **/
	/* Retrieve contents from output file output.csv (written by ./calculate). */
	$filename = './output/output.csv';
	$file = fopen($filename, 'r') or die('Could not open file ' . $filename);
	$arr_output = fgetcsv($file);
	fclose($file);
} else {
	/** We are in trouble. **/
	/* Notify the user of the problem. */
	// Read the appropriate message from the message file.
	$filename = './log/msg.txt';
	//$msg = file_get_contents($filename);
	$msg = '<p><b>ERROR:</b> An error was encountered while performing the calculations with the data submitted by you. Please check the data and make sure that all the inputs are valid.</p>';
	$msg .= '<p>If you feel that the data submitted by you is valid and still the calculations are not being performed, please preserve the data and contact the Dairy Management team regarding this. We will get back to you as soon as we can.</p>';
	$arr_output = array('iter_sum_no_match' => 1, 'msg' => $msg);
}

/* Return json object. */
echo json_encode($arr_output);
?>
