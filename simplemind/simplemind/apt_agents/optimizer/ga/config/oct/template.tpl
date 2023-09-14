<!DOCTYPE html>
<html>

<head>
<style>
table.list {
	border-collapse: collapse;
}
.list {
    border: 1px solid black;
	padding: 10px;

}
}

.equ {

	border: 0px;
	padding: 10px;
	vertical-align: top;
}
th {
  text-align: left;
}

table#t01 {
	width:100%;
}

table#t01 tr:nth-child(even) {
  background-color: #eee;
}
table#t01 tr:nth-child(odd) {
 background-color: #fff;
}

table#t01 th {
 background-color: #fff;
}


table#t02 tr:nth-child(even) {
  background-color: #fff;
}
table#t02 tr:nth-child(odd) {
 background-color: #fff;
}

</style>
</head>

<body>

<h1>
{{ gene }} <br>
{{ gene_binary }}
</h1>

<h3>
Fitness: {{ final_result["fitness"] }}
</h3>

<!-- <img width=100% src=" to_html(final_result["error_hist_plot"]) " > -->


<h1>
Overall Performance Summary
</h1>

<table class="equ" id="t01">
<!-- 	<col width="200">
	<col width="175">
	<col width="175">
	<col width="175">
	<col width="175"> -->
	<tr class="equ">
		<th class="equ" width=120px> </th>
		<th class="equ">Iris</th>
		<th class="equ">Cornea</th>
		<!-- <th class="equ">GE Junction</th>
		<th class="equ">ETT Tip</th>
		<th class="equ">NG Tube</th> -->
	</tr>

	<tr class="equ">
		<th class="equ">[Dice Coefficient]</th>
		<td class="equ"></td>
		<td class="equ"></td>
	</tr>
	<tr class="equ">
		<th class="equ">mean</th>
		<td class="equ">{{ final_result["IRIS"]["dice_coefficient_mean"] }}</td>
		<td class="equ">{{ final_result["CORNEA"]["dice_coefficient_mean"] }}</td>
	</tr>
	<tr class="equ">
		<th class="equ">std</th>
		<td class="equ">{{ final_result["IRIS"]["dice_coefficient_std"] }}</td>
		<td class="equ">{{ final_result["CORNEA"]["dice_coefficient_std"] }}</td>
	</tr>
	<tr class="equ">
		<th class="equ">median</th>
		<td class="equ">{{ final_result["IRIS"]["dice_coefficient_median"] }}</td>
		<td class="equ">{{ final_result["CORNEA"]["dice_coefficient_median"] }}</td>
	</tr>
	<tr class="equ">
		<th class="equ">1st/3rd quartiles</th>
		<td class="equ">{{ final_result["IRIS"]["dice_coefficient_q1"] }}, {{ final_result["IRIS"]["dice_coefficient_q3"] }}</td>
		<td class="equ">{{ final_result["CORNEA"]["dice_coefficient_q1"] }}, {{ final_result["CORNEA"]["dice_coefficient_q3"] }}</td>
	</tr>
	<tr class="equ">
		<th class="equ">Number annotated cases:</th>
		<td class="equ">{{ final_result["IRIS"]["num_annotated"] }}</td>
		<td class="equ">{{ final_result["CORNEA"]["num_annotated"] }}</td>
	</tr>
	<tr class="equ">
		<th class="equ">Number good predictions:</th>
		<td class="equ">{{ final_result["IRIS"]["num_good_markings"] }} cases with meeting {{ final_result["IRIS"]["threshold"] }} threshold</td>
		<td class="equ">{{ final_result["CORNEA"]["num_good_markings"] }} cases with meeting {{ final_result["CORNEA"]["threshold"] }} threshold</td>
	</tr>
	<!-- <tr class="equ">
		<td class="equ">Bad markings:</td>
		<td class="equ">{{ final_result["IRIS"]["num_bad_markings"] }}</td>
		<td class="equ">{{ final_result["CORNEA"]["num_bad_markings"] }}</td>
	</tr> -->
	<!-- <tr class="equ">
		<th class="equ">Detection:</th>
		<td class="equ">
			<table class="list" id="t02">
				<tr class="list">
					<td class="list">TP: {{ final_result["IRIS"]["tp"] }}</td>
					<td class="list">FP: {{ final_result["IRIS"]["fp"] }}</td>
				</tr>
				<tr class="list">
					<td class="list">FN: {{ final_result["IRIS"]["fn"] }}</td>
					<td class="list">TN: {{ final_result["IRIS"]["tn"] }}</td>
				</tr>
			</table>

		</td>
	</tr> -->

	<tr class="equ">
		<th class="equ">Sensitivity:</th>
		<td class="equ">{{ final_result["IRIS"]["sensitivity"] }}</td>
		<td class="equ">{{ final_result["CORNEA"]["sensitivity"] }}</td>
	</tr>
	<tr class="equ">
		<th class="equ">Specificity:</th>
		<td class="equ">{{ final_result["IRIS"]["specificity"] }}</td>
		<td class="equ">{{ final_result["CORNEA"]["specificity"] }}</td>
	</tr>

</table>

<!-- 
# - ref present/absent
# - MIU output present/absent
# - TP/FP/FN/TN
# - DCE Score
# - DCE -->


{% if final_result["bad_cases"]["IRIS"] -%}
<h1>High error cases</h1>
<table class="list">
	<tr class="list">
		<th class="list">Case</th>
		<th class="list">Iris DCE</th>
		<th class="list">Cornea DCE</th>
		<!-- <th class="list">Detection</th> -->
	</tr>
{% for key in final_result["bad_cases"]["IRIS"] -%}
	<tr class="list">
		<td class="list"><a href="sub_report/{{key}}.html">{{key}}</a></td>
		<td class="list">{{final_result["per_case_result"][key]["IRIS"].get("dice_coefficient")}}</td>
		<td class="list">{{final_result["per_case_result"][key]["CORNEA"].get("dice_coefficient")}}</td>
		<!-- <td class="list">{{final_result["per_case_result"][key]["IRIS"].get("class")}}</td> -->
	</tr>
{%- endfor %}
</table>
{%- endif %}


{% if final_result["good_cases"]["IRIS"] -%}
<h1>Other cases</h1>
<table class="list">
	<tr class="list">
		<th class="list">Case</th>
		<th class="list">Iris DCE</th>
		<th class="list">Cornea DCE</th>
		<!-- <th class="list">Detection</th> -->
	</tr>
{% for key in final_result["good_cases"]["IRIS"] -%}
	<tr class="list">
		<td class="list"><a href="sub_report/{{key}}.html">{{key}}</a></td>
		<td class="list">{{final_result["per_case_result"][key]["IRIS"].get("dice_coefficient")}}</td>
		<td class="list">{{final_result["per_case_result"][key]["CORNEA"].get("dice_coefficient")}}</td>
		<!-- <td class="list">{{final_result["per_case_result"][key]["IRIS"].get("class")}}</td> -->
	</tr>
{%- endfor %}
</table>


{% else -%}
<h1>All cases</h1>
<table class="list">
	<tr class="list">
		<th class="list">Case</th>
		<th class="list">Iris DCE</th>
		<th class="list">Cornea DCE</th>
		<!-- <th class="list">Detection</th> -->
	</tr>
{% for key in keys -%}
	<tr class="list">
		<td class="list"><a href="sub_report/{{key}}.html">{{key}}</a></td>
		<td class="list">{{final_result["per_case_result"][key]["IRIS"].get("dice_coefficient")}}</td>
		<td class="list">{{final_result["per_case_result"][key]["CORNEA"].get("dice_coefficient")}}</td>
		<!-- <td class="list">{{final_result["per_case_result"][key]["IRIS"].get("class")}}</td> -->
	</tr>
{%- endfor %}
</table>
{%- endif %}


</body>

</html>