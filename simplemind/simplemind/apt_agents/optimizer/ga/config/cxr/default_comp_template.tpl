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


<h2>Case Results Summary</h2>
<table class="list">
{% for key in keys -%}
	<tr class="list">
		<td class="list">
			<a href="sub_report/{{key}}.html">{{key}}</a>
			<br>
			{{ final_result['per_case_result'][key]['patient_id'] }}
			<br>
			{{ final_result['per_case_result'][key]['initials'] }}
		</td>
		<td class="list">
			<img width=800px src="file:/{{ final_result['per_case_result'][key]['marking_ss'].replace('\\', '//') }}" >
		</td>
	</tr>
{%- endfor %}
</table>


</body>

</html>