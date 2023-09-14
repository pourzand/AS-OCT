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




<img width=60% src="{{ to_html(per_case_result["visuals"]["preprocessed_ss"]) }}" >
<img width=25% src="{{ to_html(per_case_result["visuals"]["marking_ss"]) }}" onmouseover="this.src='{{ to_html(per_case_result["visuals"]["original_ss"]) }}';" onmouseout="this.src='{{ to_html(per_case_result["visuals"]["marking_ss"]) }}';" >


<h2> Summary </h2>
<table class="equ">
	<tr class="equ">
		<th class="equ"> </th>
		<th class="equ">Prostate </th>
	</tr>

	<tr class="equ">
		<td class="equ">DCE:</td><td class="equ">{{ per_case_result["Prost"].get("dice_coefficient", "---") }}</td>
		<td class="equ">ASSD:</td><td class="equ">{{ per_case_result["Prost"].get("assd", "---") }}</td>
		<td class="equ">HD:</td><td class="equ">{{ per_case_result["Prost"].get("hd", "---") }}</td>
		<td class="equ">HD95:</td><td class="equ">{{ per_case_result["Prost"].get("hd95", "---") }}</td>
		<td class="equ">Precision:</td><td class="equ">{{ per_case_result["Prost"].get("precision", "---") }}</td>
		<td class="equ">Recall:</td><td class="equ">{{ per_case_result["Prost"].get("recall", "---") }}</td>
		<td class="equ">Sensitivity:</td><td class="equ">{{ per_case_result["Prost"].get("sensitivity", "---") }}</td>
		<td class="equ">Specificity:</td><td class="equ">{{ per_case_result["Prost"].get("specificity", "---") }}</td>
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
	<tr class="equ">
		<td class="equ">Spacing:</td>
		<td class="equ">{{ per_case_result["spacing"]}} </td>
	</tr>
</table>

<!--
<h3> Preprocessed Images </h3>
<img width=80% src="{{ to_html(per_case_result["visuals"]["preprocessed_ss"]) }}" >
-->

</body>

</html>