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



{% for pp_ss in per_case_result["visuals"]["preprocessed_ss"] -%}
<img width=45% src="{{ to_html(pp_ss) }}" >

{%- endfor %}

<img width=45% src="{{ to_html(per_case_result["visuals"]["marking_ss"]) }}" onmouseover="this.src='{{ to_html(per_case_result["visuals"]["original_ss"]) }}';" onmouseout="this.src='{{ to_html(per_case_result["visuals"]["marking_ss"]) }}';" >


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
		<td class="equ">x error:</td>
		<td class="equ">{{ per_case_result["Crina"].get("x_err", "---") }} mm</td>
		<td class="equ">{{ per_case_result["GEjct"].get("x_err", "---") }} mm</td>
		<td class="equ">{{ per_case_result["EtTip"].get("x_err", "---") }} mm</td>
		<td class="equ">{{ per_case_result["NgTub"].get("x_err", "---") }} mm</td>
	</tr>
	<tr class="equ">
		<td class="equ">y error:</td>
		<td class="equ">{{ per_case_result["Crina"].get("y_err", "---") }} mm</td>
		<td class="equ">{{ per_case_result["GEjct"].get("y_err", "---") }} mm</td>
		<td class="equ">{{ per_case_result["EtTip"].get("y_err", "---") }} mm</td>
		<td class="equ">{{ per_case_result["NgTub"].get("y_err", "---") }} mm</td>
	</tr>
	<tr class="equ">
		<td class="equ">total error:</td>
		<td class="equ">{{ per_case_result["Crina"].get("tot_err", "---") }} mm</td>
		<td class="equ">{{ per_case_result["GEjct"].get("tot_err", "---") }} mm</td>
		<td class="equ">{{ per_case_result["EtTip"].get("tot_err", "---") }} mm</td>
		<td class="equ">{{ per_case_result["NgTub"].get("tot_err", "---") }} mm</td>
	</tr>
	<tr class="equ">
		<td class="equ">Detection:</td>
		<td class="equ">{{ per_case_result["Crina"].get("class", "---") }} mm</td>
		<td class="equ">{{ per_case_result["GEjct"].get("class", "---") }} mm</td>
		<td class="equ">{{ per_case_result["EtTip"].get("class", "---") }} mm</td>
		<td class="equ">{{ per_case_result["NgTub"].get("class", "---") }} mm</td>
	</tr>
	<tr class="equ">
		<td class="equ">Pred Detection:</td>
		<td class="equ">{{ per_case_result["Crina"].get("pred_detection", "---") }} mm</td>
		<td class="equ">{{ per_case_result["GEjct"].get("pred_detection", "---") }} mm</td>
		<td class="equ">{{ per_case_result["EtTip"].get("pred_detection", "---") }} mm</td>
		<td class="equ">{{ per_case_result["NgTub"].get("pred_detection", "---") }} mm</td>
	</tr>
	<tr class="equ">
		<td class="equ">Ref Detection:</td>
		<td class="equ">{{ per_case_result["Crina"].get("ref_detection", "---") }} mm</td>
		<td class="equ">{{ per_case_result["GEjct"].get("ref_detection", "---") }} mm</td>
		<td class="equ">{{ per_case_result["EtTip"].get("ref_detection", "---") }} mm</td>
		<td class="equ">{{ per_case_result["NgTub"].get("ref_detection", "---") }} mm</td>
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