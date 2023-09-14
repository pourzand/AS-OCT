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




<img width=45% src="{{ to_html(per_case_result["clahe_ss"]) }}" >
<img width=45% src="{{ to_html(per_case_result["display_ss"]) }}" >
<img width=45% src="{{ to_html(per_case_result["original_ss"]) }}" >
<img width=45% src="{{ to_html(per_case_result["marking_ss"]) }}" >


<h2> Summary </h2>
<table class="equ">
	<tr class="equ">
		<th class="equ"> </th>
		<th class="equ">Carina </th>
		<th class="equ">GE Junction </th>
		<th class="equ">ETT Tip </th>
		<th class="equ">NG Tube </th>

	</tr>
	<tr class="equ">
		<td class="equ">Error [mm]:</td>
		<td class="equ">{{ per_case_result["Crina"].get("tot_err", "---") }} mm</td>
		<td class="equ">{{ per_case_result["GEjct"].get("tot_err", "---") }} mm</td>
		<td class="equ">{{ per_case_result["EtTip"].get("tot_err", "---") }} mm</td>
		<td class="equ">{{ per_case_result["NgTub"].get("tot_err", "---") }} mm</td>
	</tr>
	<tr class="equ">
		<td class="equ">Detection:</td>
		<td class="equ">{{ per_case_result["Crina"].get("class", "---") }}</td>
		<td class="equ">{{ per_case_result["GEjct"].get("class", "---") }}</td>
		<td class="equ">{{ per_case_result["EtTip"].get("class", "---") }}</td>
		<td class="equ">{{ per_case_result["NgTub"].get("class", "---") }}</td>
	</tr>
	<tr class="equ">
		<td class="equ">Marking note:</td>
		<td class="equ">{{ per_case_result["Crina"].get("markup", "---") }}</td>
		<td class="equ">{{ per_case_result["GEjct"].get("markup", "---") }}</td>
		<td class="equ">{{ per_case_result["EtTip"].get("markup", "---") }}</td>
		<td class="equ">{{ per_case_result["NgTub"].get("markup", "---") }}</td>
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