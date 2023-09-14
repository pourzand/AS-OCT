<!DOCTYPE html>
<html>

<head>
<style>
table {
	width:800px;
}
th {
  text-align: left;
}


table.list {
	border-collapse: collapse;
}
.list {
    border: 1px solid black;
}

.equ {
	vertical-align: top;
	padding: 10px;
	border: 0px;
}

.swap img {
	width: auto;
	height: auto;
}

.inline-block {
   display: inline-block;
}
.swap img.org{display:none}
.swap:hover img.overlay{display:none}
.swap:hover img.org{display:inline-block}
</style>
</head>

<body>

<h1>
{{ key }}
</h1>


(orig | iris | cornea)
<br>
{{per_case_result["visuals"]}}
<img width=30% src="{{ to_html(per_case_result["visuals"].get("IRIS", dict(original_ss="fake/path", output_ss="fake/path"))["original_ss"]) }}" >
<img width=30% src="{{ to_html(per_case_result["visuals"].get("IRIS", dict(original_ss="fake/path", output_ss="fake/path"))["output_ss"]) }}" onmouseover="this.src='{{ to_html(per_case_result["visuals"].get("IRIS", dict(original_ss="fake/path", annotation_ss="fake/path"))["annotation_ss"]) }}';" onmouseout="this.src='{{ to_html(per_case_result["visuals"].get("IRIS", dict(original_ss="fake/path", output_ss="fake/path"))["output_ss"]) }}';" >
<img width=30% src="{{ to_html(per_case_result["visuals"].get("CORNEA", dict(original_ss="fake/path", output_ss="fake/path"))["output_ss"]) }}" onmouseover="this.src='{{ to_html(per_case_result["visuals"].get("CORNEA", dict(original_ss="fake/path", annotation_ss="fake/path"))["annotation_ss"]) }}';" onmouseout="this.src='{{ to_html(per_case_result["visuals"].get("CORNEA", dict(original_ss="fake/path", output_ss="fake/path"))["output_ss"]) }}';" >

<h2> Summary </h2>
<table class="equ">
	<tr class="equ">
		<th class="equ"> </th>
		<th class="equ">Iris </th>
		<th class="equ">Cornea </th>

	</tr>

	<tr class="equ">
		<td class="equ">DCE:</td>
		<td class="equ">{{ per_case_result["IRIS"].get("dice_coefficient", "---") }}</td>
		<td class="equ">{{ per_case_result["CORNEA"].get("dice_coefficient", "---") }}</td>
	</tr>

</table>

<h3> Markup Information </h3>
<table class="equ">
	<tr class="equ">
		<td class="equ">Patient ID:</td>
		<td class="equ">{{ per_case_result["patient_id"]}} </td>
	</tr>
	<tr class="equ">
		<td class="equ">Initials:</td>
		<td class="equ">{{ per_case_result["initials"]}} </td>
	</tr>
</table>

</body>

</html>
