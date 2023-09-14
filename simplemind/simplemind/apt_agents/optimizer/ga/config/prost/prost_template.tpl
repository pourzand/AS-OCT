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
Overall Performance Summary (Prostate)
</h1>

<table class="equ" id="t01">
<!-- 	<col width="200">
	<col width="175">
	<col width="175">
	<col width="175">
	<col width="175"> -->
	<tr class="equ">
		<th class="equ" width=120px> </th>
		<th class="equ">DCE Score</th>
		<th class="equ">Dice Coefficient</th>
		<th class="equ">ASSD</th>
		<th class="equ">HD</th>
		<th class="equ">HD95</th>
		<th class="equ">Precision</th>
		<th class="equ">Recall</th>
		<th class="equ">Voxelwise Sensitivity</th>
		<th class="equ">Voxelwise Specificity</th>
		<!-- <th class="equ">GE Junction</th>
		<th class="equ">ETT Tip</th>
		<th class="equ">NG Tube</th> -->
	</tr>
<!-- 
	<tr class="equ">
		<th class="equ">[DCE Score]</th>
		<td class="equ"></td>
	</tr>
-->
	<tr class="equ">
		<th class="equ">mean</th>
		<td class="equ">{{ final_result["Prost"]["dce_score_mean"] }}</td>
		<td class="equ">{{ final_result["Prost"]["dice_coefficient_mean"] }}</td>
		<td class="equ">{{ final_result["Prost"]["assd_mean"] }}</td>
		<td class="equ">{{ final_result["Prost"]["hd_mean"] }}</td>
		<td class="equ">{{ final_result["Prost"]["hd95_mean"] }}</td>
		<td class="equ">{{ final_result["Prost"]["precision_mean"] }}</td>
		<td class="equ">{{ final_result["Prost"]["recall_mean"] }}</td>
		<td class="equ">{{ final_result["Prost"]["voxelwise_sensitivity_mean"] }}</td>
		<td class="equ">{{ final_result["Prost"]["voxelwise_specificity_mean"] }}</td>
	</tr>
	<tr class="equ">
		<th class="equ">std</th>
		<td class="equ">{{ final_result["Prost"]["dce_score_std"] }}</td>
		<td class="equ">{{ final_result["Prost"]["dice_coefficient_std"] }}</td>
		<td class="equ">{{ final_result["Prost"]["assd_std"] }}</td>
		<td class="equ">{{ final_result["Prost"]["hd_std"] }}</td>
		<td class="equ">{{ final_result["Prost"]["hd95_std"] }}</td>
		<td class="equ">{{ final_result["Prost"]["precision_std"] }}</td>
		<td class="equ">{{ final_result["Prost"]["recall_std"] }}</td>
		<td class="equ">{{ final_result["Prost"]["voxelwise_sensitivity_std"] }}</td>
		<td class="equ">{{ final_result["Prost"]["voxelwise_specificity_std"] }}</td>
	</tr>
	<tr class="equ">
		<th class="equ">median</th>
		<td class="equ">{{ final_result["Prost"]["dce_score_median"] }}</td>
		<td class="equ">{{ final_result["Prost"]["dice_coefficient_median"] }}</td>
		<td class="equ">{{ final_result["Prost"]["assd_median"] }}</td>
		<td class="equ">{{ final_result["Prost"]["hd_median"] }}</td>
		<td class="equ">{{ final_result["Prost"]["hd95_median"] }}</td>
		<td class="equ">{{ final_result["Prost"]["precision_median"] }}</td>
		<td class="equ">{{ final_result["Prost"]["recall_median"] }}</td>
		<td class="equ">{{ final_result["Prost"]["voxelwise_sensitivity_median"] }}</td>
		<td class="equ">{{ final_result["Prost"]["voxelwise_specificity_median"] }}</td>
	</tr>
	<tr class="equ">
		<th class="equ">1st/3rd quartiles</th>
		<td class="equ">{{ final_result["Prost"]["dce_score_q1"] }}, {{ final_result["Prost"]["dce_score_q3"] }}</td>
		<td class="equ">{{ final_result["Prost"]["dice_coefficient_q1"] }}, {{ final_result["Prost"]["dice_coefficient_q3"] }}</td>
		<td class="equ">{{ final_result["Prost"]["assd_q1"] }}, {{ final_result["Prost"]["assd_q3"] }}</td>
		<td class="equ">{{ final_result["Prost"]["hd_q1"] }}, {{ final_result["Prost"]["hd_q3"] }}</td>
		<td class="equ">{{ final_result["Prost"]["hd95_q1"] }}, {{ final_result["Prost"]["hd95_q3"] }}</td>
		<td class="equ">{{ final_result["Prost"]["precision_q1"] }}, {{ final_result["Prost"]["precision_q3"] }}</td>
		<td class="equ">{{ final_result["Prost"]["recall_q1"] }}, {{ final_result["Prost"]["recall_q3"] }}</td>
		<td class="equ">{{ final_result["Prost"]["voxelwise_sensitivity_q1"] }}, {{ final_result["Prost"]["voxelwise_sensitivity_q3"] }}</td>
		<td class="equ">{{ final_result["Prost"]["voxelwise_specificity_q1"] }}, {{ final_result["Prost"]["voxelwise_specificity_q3"] }}</td>
	</tr>
	<tr class="equ">
		<th class="equ">Number annotated cases:</th>
		<td class="equ">{{ final_result["Prost"]["num_annotated"] }}</td>
	</tr>
	<tr class="equ">
		<th class="equ">Number good predictions:</th>
		<td class="equ">{{ final_result["Prost"]["num_good_markings"] }} cases with meeting {{ final_result["Prost"]["threshold"] }} threshold</td>
	</tr>
	<!-- <tr class="equ">
		<td class="equ">Bad markings:</td>
		<td class="equ">{{ final_result["Prost"]["num_bad_markings"] }}</td>
	</tr> -->
	<tr class="equ">
		<th class="equ">Detection:</th>
		<td class="equ">
			<table class="list" id="t02">
				<tr class="list">
					<td class="list">TP: {{ final_result["Prost"]["tp"] }}</td>
					<td class="list">FP: {{ final_result["Prost"]["fp"] }}</td>
				</tr>
				<tr class="list">
					<td class="list">FN: {{ final_result["Prost"]["fn"] }}</td>
					<td class="list">TN: {{ final_result["Prost"]["tn"] }}</td>
				</tr>
			</table>

		</td>
	</tr>

	<tr class="equ">
		<th class="equ">Sensitivity:</th>
		<td class="equ">{{ final_result["Prost"]["sensitivity"] }}</td>
	</tr>
	<tr class="equ">
		<th class="equ">Specificity:</th>
		<td class="equ">{{ final_result["Prost"]["specificity"] }}</td>
	</tr>

</table>

<!-- 
# - ref present/absent
# - MIU output present/absent
# - TP/FP/FN/TN
# - DCE Score
# - DCE -->


{% if final_result["bad_cases"]["Prost"] -%}
<h1>High error cases</h1>
<table class="list">
	<tr class="list">
		<th class="list">Case</th>
		<th class="list">DCE Score</th>
		<th class="list">DCE</th>
		<th class="list">ASSD</th>
		<th class="list">HD</th>
		<th class="list">HD95</th>
		<th class="list">Precision</th>
		<th class="list">Recall</th>
		<th class="list">Sensitivity</th>
		<th class="list">Specificity</th>
		<th class="list">Detection</th>
	</tr>
{% for key in final_result["bad_cases"]["Prost"] -%}
	<tr class="list">
		<td class="list"><a href="sub_report/{{key}}.html">{{key}}</a></td>
		<td class="list">{{final_result["per_case_result"][key]["Prost"].get("dce_score")}}</td>
		<td class="list">{{final_result["per_case_result"][key]["Prost"].get("dice_coefficient")}}</td>
		<td class="list">{{final_result["per_case_result"][key]["Prost"].get("assd")}}</td>
		<td class="list">{{final_result["per_case_result"][key]["Prost"].get("hd")}}</td>
		<td class="list">{{final_result["per_case_result"][key]["Prost"].get("hd95")}}</td>
		<td class="list">{{final_result["per_case_result"][key]["Prost"].get("precision")}}</td>
		<td class="list">{{final_result["per_case_result"][key]["Prost"].get("recall")}}</td>
		<td class="list">{{final_result["per_case_result"][key]["Prost"].get("sensitivity")}}</td>
		<td class="list">{{final_result["per_case_result"][key]["Prost"].get("specificity")}}</td>
		<td class="list">{{final_result["per_case_result"][key]["Prost"].get("class")}}</td>
	</tr>
{%- endfor %}
</table>
{%- endif %}


<!-- {% if final_result["misplaced"] -%}
<h1>Incorrect placement cases</h1>
<table class="list">
	<tr class="list">
		<th class="list">Case</th>
		<th class="list">Carina Error</th>
		<th class="list">GE Junction Error</th>
		<th class="list">ETT Tip Error</th>
		<th class="list">NG Tube Error</th>
		<th class="list">ET Correct</th>
		<th class="list">NG Correct</th>
	</tr>
{% for key in final_result["misplaced"] -%}
	<tr class="list">
		<td class="list"><a href="sub_report/{{key}}.html">{{key}}</a></td>
		<td class="list">{{final_result["per_case_result"][key]["Crina"].get("tot_err", final_result["per_case_result"][key]["Crina"]["class"])}}</td>
		<td class="list">{{final_result["per_case_result"][key]["GEjct"].get("tot_err", final_result["per_case_result"][key]["GEjct"]["class"])}}</td>
		<td class="list">{{final_result["per_case_result"][key]["EtTip"].get("tot_err", final_result["per_case_result"][key]["EtTip"]["class"])}}</td>
		<td class="list">{{final_result["per_case_result"][key]["NgTub"].get("tot_err", final_result["per_case_result"][key]["NgTub"]["class"])}}</td>
		<td class="list">{{final_result["per_case_result"][key]["EtTip"].get("correct")}}</td>
		<td class="list">{{final_result["per_case_result"][key]["NgTub"].get("correct")}}</td>
	</tr>
{%- endfor %}
</table>
{%- endif %} -->


{% if final_result["good_cases"]["Prost"] -%}
<h1>Other cases</h1>
<table class="list">
	<tr class="list">
		<th class="list">Case</th>
		<th class="list">DCE Score</th>
		<th class="list">DCE</th>
		<th class="list">ASSD</th>
		<th class="list">HD</th>
		<th class="list">HD95</th>
		<th class="list">Precision</th>
		<th class="list">Recall</th>
		<th class="list">Sensitivity</th>
		<th class="list">Specificity</th>
		<th class="list">Detection</th>
	</tr>
{% for key in final_result["good_cases"]["Prost"] -%}
	<tr class="list">
		<td class="list"><a href="sub_report/{{key}}.html">{{key}}</a></td>
		<td class="list">{{final_result["per_case_result"][key]["Prost"].get("dce_score")}}</td>
		<td class="list">{{final_result["per_case_result"][key]["Prost"].get("dice_coefficient")}}</td>
		<td class="list">{{final_result["per_case_result"][key]["Prost"].get("assd")}}</td>
		<td class="list">{{final_result["per_case_result"][key]["Prost"].get("hd")}}</td>
		<td class="list">{{final_result["per_case_result"][key]["Prost"].get("hd95")}}</td>
		<td class="list">{{final_result["per_case_result"][key]["Prost"].get("precision")}}</td>
		<td class="list">{{final_result["per_case_result"][key]["Prost"].get("recall")}}</td>
		<td class="list">{{final_result["per_case_result"][key]["Prost"].get("sensitivity")}}</td>
		<td class="list">{{final_result["per_case_result"][key]["Prost"].get("specificity")}}</td>
		<td class="list">{{final_result["per_case_result"][key]["Prost"].get("class")}}</td>
	</tr>
{%- endfor %}
</table>


{% else -%}
<h1>All cases</h1>
<table class="list">
	<tr class="list">
		<th class="list">Case</th>
		<th class="list">DCE Score</th>
		<th class="list">DCE</th>
		<th class="list">ASSD</th>
		<th class="list">HD</th>
		<th class="list">HD95</th>
		<th class="list">Precision</th>
		<th class="list">Recall</th>
		<th class="list">Sensitivity</th>
		<th class="list">Specificity</th>
		<th class="list">Detection</th>
		<th class="list">Initials</th>
	</tr>
{% for key in keys -%}
	<tr class="list">
		<td class="list"><a href="sub_report/{{key}}.html">{{key}}</a></td>
		<td class="list">{{final_result["per_case_result"][key]["Prost"].get("dce_score")}}</td>
		<td class="list">{{final_result["per_case_result"][key]["Prost"].get("dice_coefficient")}}</td>
		<td class="list">{{final_result["per_case_result"][key]["Prost"].get("assd")}}</td>
		<td class="list">{{final_result["per_case_result"][key]["Prost"].get("hd")}}</td>
		<td class="list">{{final_result["per_case_result"][key]["Prost"].get("hd95")}}</td>
		<td class="list">{{final_result["per_case_result"][key]["Prost"].get("precision")}}</td>
		<td class="list">{{final_result["per_case_result"][key]["Prost"].get("recall")}}</td>
		<td class="list">{{final_result["per_case_result"][key]["Prost"].get("sensitivity")}}</td>
		<td class="list">{{final_result["per_case_result"][key]["Prost"].get("specificity")}}</td>
		<td class="list">{{final_result["per_case_result"][key]["Prost"].get("class")}}</td>
		<td class="list">{{final_result["per_case_result"].get("initial")}}</td>
	</tr>
{%- endfor %}
</table>
{%- endif %}


</body>

</html>