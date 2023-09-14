<!DOCTYPE html>
<html>

<head>
<style>
table.list {
	border-collapse: collapse;
}
.list {
    border: 1px solid black;
}

.equ {
	vertical-align: top;
	border: 0px;
}

</style>
</head>

<body>

<h1>
{{ gene }}
</h1>

<h2>Fitness</h2>
<table class="equ">
	<tr class="equ">
		<td class="equ">Sensitivity:</td>
		<td class="equ">{{ final_result["sensitivity"] }}</td>
	</tr>
	<tr class="equ">
		<td class="equ">Normalized FP:</td>
		<td class="equ">{{ final_result["normalized_fp"] }}</td>
	</tr>
	<tr class="equ">
		<td class="equ">Overlap ratio:</td>
		<td class="equ">{{ final_result["overlap_ratio"] }}</td>
	</tr>
	<tr class="equ">
		<td class="equ">Fitness:</td>
		<td class="equ">{{ final_result["formula"] }}<br>= {{ final_result["fitness"] }}</td>
	</tr>
</table>

<h2>Nodule level statistics</h2>
<table class="equ">
	<tr class="equ">
		<td class="equ">Total number of nodules:</td>
		<td class="equ">{{ final_result["nodule_num"] }}</td>
	</tr>
	<tr class="equ">
		<td class="equ">Total true positives:</td>
		<td class="equ">{{ final_result["total_tp"] }}</td>
	</tr>
	<tr class="equ">
		<td class="equ">Sensitivity:</td>
		<td class="equ">{{ final_result["nodule_sensitivity"] }}</td>
	</tr>
</table>

<h2>Case level statistics</h2>
<h3>Sensitivity</h3>
<table class="equ">
	<tr class="equ">
		<td class="equ">N:</td>
		<td class="equ">{{ final_result["sensitivity_n"] }}</td>
	</tr>
	<tr class="equ">
		<td class="equ">Mean:</td>
		<td class="equ">{{ final_result["sensitivity_mean"] }}</td>
	</tr>
	<tr class="equ">
		<td class="equ">Standard deviation:</td>
		<td class="equ">{{ final_result["sensitivity_std"] }}</td>
	</tr>
	<tr class="equ">
		<td class="equ">Median:</td>
		<td class="equ">{{ final_result["sensitivity_median"] }}</td>
	</tr>
	<tr class="equ">
		<td class="equ">Q1:</td>
		<td class="equ">{{ final_result["sensitivity_q1"] }}</td>
	</tr>
	<tr class="equ">
		<td class="equ">Q3:</td>
		<td class="equ">{{ final_result["sensitivity_q3"] }}</td>
	</tr>
</table>
<h3>False positives</h3>
<table class="equ">
	<tr class="equ">
		<td class="equ">Number of cases:</td>
		<td class="equ">{{ final_result["fp_n"] }}</td>
	</tr>
	<tr class="equ">
		<td class="equ">Mean:</td>
		<td class="equ">{{ final_result["fp_mean"] }}</td>
	</tr>
	<tr class="equ">
		<td class="equ">Standard deviation:</td>
		<td class="equ">{{ final_result["fp_std"] }}</td>
	</tr>
	<tr class="equ">
		<td class="equ">Median:</td>
		<td class="equ">{{ final_result["fp_median"] }}</td>
	</tr>
	<tr class="equ">
		<td class="equ">Q1:</td>
		<td class="equ">{{ final_result["fp_q1"] }}</td>
	</tr>
	<tr class="equ">
		<td class="equ">Q3:</td>
		<td class="equ">{{ final_result["fp_q3"] }}</td>
	</tr>
</table>


<h2>Case results</h2>
<table class="list">
	<tr class="list">
		<th class="list">Case</th>
		<th class="list">TP</th>
		<th class="list">FP</th>
		<th class="list">FN</th>
		<th class="list">Sensitivity</th>
		<th class="list">Normalized FP</th>
		<th class="list">Overlap ratio</th>
		<th class="list">Slice spacing</th>
	</tr>
{% for key in keys -%}
	<tr class="list">
		<td class="list"><a href="sub_report/{{key}}.html">{{key}}</a></td>
		<td class="list">{{final_result["per_case_result"][key]["tp"]}}</td>
		<td class="list">{{final_result["per_case_result"][key]["fp"]}}</td>
		<td class="list">{{final_result["per_case_result"][key]["fn"]}}</td>
		<td class="list">{{final_result["per_case_result"][key]["sensitivity"]}}</td>
		<td class="list">{{final_result["per_case_result"][key]["normalized_fp"]}}</td>
		<td class="list">{{final_result["per_case_result"][key]["overlap_ratio"]}}</td>
		<td class="list">{{result_dictionary[key].get("overall", {}).get("spacing", ["N/A", "N/A", "N/A"])[2]}}</td>
	</tr>
{%- endfor %}
</table>


<table 

</body>

</html>